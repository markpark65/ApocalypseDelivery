#include "ApocalypseGameMode.h"
#include "MeteorSpawner.h"
#include "Drone.h"
#include "PackageSpawner.h"
#include "DeliveryPlatform.h"
#include "ApocalypseHUD.h"
#include "ItemSpawner.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerStart.h"
#include "Components/AudioComponent.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

AApocalypseGameMode::AApocalypseGameMode() { PrimaryActorTick.bCanEverTick = true; }

void AApocalypseGameMode::BeginPlay()
{
    Super::BeginPlay();
    CurrentTimeLeft = TimeLimit;
    bIsTimerActive = false;
    if (BackgroundMusic)
    {
        // 2D 사운드로 재생
        BGMComponent = UGameplayStatics::SpawnSound2D(this, BackgroundMusic);
    }
    if (HUDWidgetClass)
    {
        CurrentHUD = CreateWidget<UApocalypseHUD>(GetWorld(), HUDWidgetClass);
        if (CurrentHUD)
        {
            CurrentHUD->AddToViewport();
            // 초기값 설정
            CurrentHUD->UpdateStatus(CurrentStage, CurrentWave, DeliveredCount, TargetDeliveries);

            int32 Mins = FMath::FloorToInt(CurrentTimeLeft / 60.0f);
            int32 Secs = FMath::FloorToInt(CurrentTimeLeft) % 60;
            CurrentHUD->UpdateTimer(Mins, Secs);
        }
    }
    PlayerDrone = Cast<ADrone>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
        // 레벨에 배치된 Spawner 찾기
    Spawner = Cast<AMeteorSpawner>(UGameplayStatics::GetActorOfClass(GetWorld(), AMeteorSpawner::StaticClass()));
    
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        PC->bShowMouseCursor = true;
        // Game과 UI를 동시에 조작할 수 있게 설정
        FInputModeGameAndUI InputMode;
        if (CurrentHUD)
        {
            InputMode.SetWidgetToFocus(CurrentHUD->TakeWidget());
        }
        PC->SetInputMode(InputMode);
    }
    ItemSpawner = Cast<AItemSpawner>(UGameplayStatics::GetActorOfClass(GetWorld(), AItemSpawner::StaticClass()));
}

void AApocalypseGameMode::StartQuest()
{
    UpdateDifficulty();
    if (Spawner)
    {
        Spawner->StartSpawning();
        bIsTimerActive = true;
        UE_LOG(LogTemp, Warning, TEXT("Quest Started! Meteors Falling..."));
    }
    AActor* FoundSpawner = UGameplayStatics::GetActorOfClass(GetWorld(), APackageSpawner::StaticClass());
    if (APackageSpawner* PSpawner = Cast<APackageSpawner>(FoundSpawner))
    {
        PSpawner->SpawnPackage();
    }
    CurrentTargetPlatform = GetRandomAvailablePlatform();
    TArray<AActor*> AllPlatforms;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADeliveryPlatform::StaticClass(), AllPlatforms);
    for (AActor* Actor : AllPlatforms)
    {
        if (ADeliveryPlatform* Platform = Cast<ADeliveryPlatform>(Actor))
        {
            bool bIsCurrentTarget = (Platform == CurrentTargetPlatform);
            Platform->SetActorHiddenInGame(!bIsCurrentTarget);
            Platform->SetIsTarget(bIsCurrentTarget);
            if (Platform->bIsUsed)
            {
                Platform->SetActorHiddenInGame(true);
            }
        }
    }

    if (CurrentTargetPlatform)
    {
        CurrentTargetPlatform->SetIsTarget(true);
        UE_LOG(LogTemp, Warning, TEXT("New Target Platform Assigned!"));
    }
}
void AApocalypseGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (PlayerDrone)
    {
        float BatteryPct = PlayerDrone->CurrentBattery / PlayerDrone->MaxBattery;
        float SpeedValue = PlayerDrone->GetCurrentVelocity().Size();

        if (CurrentHUD)
        {
            CurrentHUD->UpdateStats(BatteryPct, SpeedValue);
        }
        if (PlayerDrone->CurrentBattery <= 0)
        {
            EndGame(false);
            PlayerDrone = nullptr;
            return;
        }
    }
    else if (bIsTimerActive)
    {
        EndGame(false);
    }
    if (bIsTimerActive && CurrentTimeLeft > 0)
    {
        CurrentTimeLeft -= DeltaTime;

        // HUD 업데이트
        if (CurrentHUD)
        {
            int32 Minutes = FMath::FloorToInt(CurrentTimeLeft / 60.0f);
            int32 Seconds = FMath::FloorToInt(CurrentTimeLeft) % 60;
            CurrentHUD->UpdateTimer(Minutes, Seconds);
        }

        // 시간 초과 시 게임 오버
        if (CurrentTimeLeft <= 0)
        {
            CurrentTimeLeft = 0;
            bIsTimerActive = false;
            EndGame(false); // 실패 처리
        }
    }

    if (PlayerDrone && CurrentHUD)
    {
        float BatteryPct = PlayerDrone->CurrentBattery / PlayerDrone->MaxBattery;

        float SpeedValue = PlayerDrone->GetCurrentVelocity().Size();

        CurrentHUD->UpdateStats(BatteryPct, SpeedValue);

        if (CurrentTargetPlatform)
        {
            // 드론과 플랫폼 사이의 거리 계산
            float Distance = FVector::Dist(PlayerDrone->GetActorLocation(), CurrentTargetPlatform->GetActorLocation());
            float DistanceInMeters = Distance / 100.0f;

            // HUD에 거리 전달
            CurrentHUD->UpdateDistance(DistanceInMeters);
        }
        else
        {
            // 타겟이 없을 때는 0이나 특정 값으로 초기화
            CurrentHUD->UpdateDistance(0.0f);
        }
    }
}

void AApocalypseGameMode::OnPackageDelivered(ADeliveryPlatform* TargetPlatform)
{
    bIsTimerActive = false;
    DeliveredCount++;
    CurrentWave++;

    if (TargetPlatform)
    {
        TargetPlatform->MarkAsUsed();
    }
    
    if (CurrentWave > MaxWavesPerStage)
    {
        CurrentStage++;
        CurrentWave = 1;
        //UpdateDifficulty();
        if (CurrentHUD)
        {
            // HUD에 스테이지 클리어 UI 호출
            CurrentHUD->ShowStageClearUI();
        }
        FTimerHandle LevelTransitionTimer;
        GetWorldTimerManager().SetTimer(LevelTransitionTimer, this, &AApocalypseGameMode::MoveToNextLevel, 3.0f, false);

        return;
    }
    if (PlayerDrone)
    {
        PlayerDrone->AddBattery(PlayerDrone->MaxBattery);
    }
    if (CurrentStage > MaxWavesPerStage) // 모든 스테이지 클리어 시
    {
        bIsTimerActive = false;
        EndGame(true);
    }

    if (PlayerDrone)
    {
        AActor* PlayerStart = UGameplayStatics::GetActorOfClass(GetWorld(), APlayerStart::StaticClass());
        if (PlayerStart)
        {
            PlayerDrone->SetActorLocationAndRotation(PlayerStart->GetActorLocation(), PlayerStart->GetActorRotation());
            if (auto* Mesh = Cast<UPrimitiveComponent>(PlayerDrone->GetRootComponent()))
            {
                Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
                Mesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
            }
        }
    }

    if(CurrentHUD)
    {
        CurrentHUD->ShowDeliverySuccessUI();
        FString StageInfo = FString::Printf(TEXT("%d - %d"), CurrentStage, CurrentWave);
        CurrentHUD->UpdateStageText(StageInfo);
        CurrentHUD->UpdateStats(1.0f, 0.0f);
        CurrentHUD->UpdateStatus(CurrentStage, CurrentWave, DeliveredCount, TargetDeliveries);
        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            PC->bShowMouseCursor = true;
            FInputModeGameAndUI InputMode;
            if (CurrentHUD)
            {
                CurrentHUD->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
                if (UWidget* ButtonWidget = Cast<UWidget>(CurrentHUD->StartQuestButton))
                {
                    ButtonWidget->SetVisibility(ESlateVisibility::Visible);
                }

                InputMode.SetWidgetToFocus(CurrentHUD->TakeWidget());
                InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            }
            PC->SetInputMode(InputMode);
            CurrentHUD->UpdateStatus(CurrentStage, CurrentWave, DeliveredCount, TargetDeliveries);
        }
    }

    CurrentTimeLeft = TimeLimit;
    int32 Mins = FMath::FloorToInt(CurrentTimeLeft / 60.0f);
    int32 Secs = FMath::FloorToInt(CurrentTimeLeft) % 60;
    CurrentHUD->UpdateTimer(Mins, Secs);
}

void AApocalypseGameMode::UpdateDifficulty()
{
    if (WaveSettings.IsValidIndex(CurrentWaveIndex))
    {
        const FWaveData& CurrentWaveData = WaveSettings[CurrentWaveIndex];

        // 시간 제한 적용
        TimeLimit = CurrentWaveData.TimeLimit;
        CurrentTimeLeft = TimeLimit;

        // 목표 배달 수 적용
        TargetDeliveries = CurrentWaveData.TargetDeliveries;

        // 운석 스패너 설정
        if (Spawner)
        {
            Spawner->SetSpawnInterval(CurrentWaveData.MeteorSpawnInterval);
            Spawner->StartSpawning();
        }

        // 지뢰 및 아이템 스폰
        if (ItemSpawner)
        {
            ItemSpawner->SpawnMines(CurrentWaveData.MineSpawnCount);
            ItemSpawner->SpawnDebuffs(CurrentWaveData.DebuffItemCount);
            ItemSpawner->SpawnBuffs(CurrentWaveData.BuffItemCount);
        }

        UE_LOG(LogTemp, Warning, TEXT("Wave %d Started! Target: %d"), CurrentWaveIndex + 1, TargetDeliveries);
    }
}

void AApocalypseGameMode::GameOver()
{
    if (Spawner) Spawner->StopSpawning();

    // UI에 게임오버 알림
    UGameplayStatics::SetGamePaused(GetWorld(), true);
}

void AApocalypseGameMode::EndGame(bool bIsVictory)
{
    UGameplayStatics::SetGamePaused(GetWorld(), true);
    if (BGMComponent)
    {
        BGMComponent->FadeOut(1.0f, 0.0f);
    }
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        PC->bShowMouseCursor = true;
        FInputModeUIOnly InputMode;
        PC->SetInputMode(InputMode);
    }

    // 승리/패배 여부에 따라 생성할 클래스 결정
    TSubclassOf<UUserWidget> WidgetToCreate = bIsVictory ? SuccessWidgetClass : FailureWidgetClass;

    if (WidgetToCreate)
    {
        UUserWidget* ResultWidget = CreateWidget<UUserWidget>(GetWorld(), WidgetToCreate);
        if (ResultWidget)
        {
            ResultWidget->AddToViewport();
        }
    }
}
void AApocalypseGameMode::MoveToNextLevel()
{
    FName NextLevelName;

    if (CurrentStage == 2)
    {
        NextLevelName = FName("Stage2_Map");
    }
    else if (CurrentStage == 3)
    {
        NextLevelName = FName("Stage3_Map");
    }
    else
    {
        NextLevelName = FName("MainMenu"); // 혹은 최종 승리 화면
    }

    UE_LOG(LogTemp, Warning, TEXT("Moving to Level: %s"), *NextLevelName.ToString());

    // 맵 이동 실행
    UGameplayStatics::OpenLevel(GetWorld(), NextLevelName);
}

ADeliveryPlatform* AApocalypseGameMode::GetRandomAvailablePlatform()
{
    TArray<AActor*> FoundPlatforms;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADeliveryPlatform::StaticClass(), FoundPlatforms);

    TArray<ADeliveryPlatform*> AvailablePlatforms;
    for (AActor* Actor : FoundPlatforms)
    {
        ADeliveryPlatform* Platform = Cast<ADeliveryPlatform>(Actor);
        if (Platform && !Platform->bIsUsed)
        {
            AvailablePlatforms.Add(Platform);
        }
    }

    if (AvailablePlatforms.Num() > 0)
    {
        return AvailablePlatforms[FMath::RandRange(0, AvailablePlatforms.Num() - 1)];
    }
    return nullptr;
}