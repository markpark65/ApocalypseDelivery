#include "ApocalypseGameMode.h"
#include "ApocalypseGameStateBase.h"
#include "MeteorSpawner.h"
#include "Drone.h"
#include "PackageSpawner.h"
#include "DeliveryPackage.h"
#include "DeliveryPlatform.h"
#include "ApocalypseHUD.h"
#include "ItemSpawner.h"


#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerStart.h"
#include "Components/AudioComponent.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include <ApocalypseGameInstance.h>

AApocalypseGameMode::AApocalypseGameMode() { PrimaryActorTick.bCanEverTick = true; }

void AApocalypseGameMode::BeginPlay()
{
    Super::BeginPlay();

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    UApocalypseGameInstance* GI = Cast<UApocalypseGameInstance>(GetGameInstance());
    //첫 실행 시 로딩화면 예외처리
    if (GI && GI->bIsFirstBoot)
    {
        GI->bIsFirstBoot = false; //다음부터는 로딩이 뜨도록 바로 false 처리

        if (PC)
        {
            PC->bShowMouseCursor = true;
            PC->SetInputMode(FInputModeUIOnly());
        }
    }
    else
    {
        if (PC && PC->PlayerCameraManager)
        {
            //다음 레벨이 불러오자마자 화면을 즉시 검은색으로 강제 유지
            PC->PlayerCameraManager->StartCameraFade(1.0f, 1.0f, 0.0f, FLinearColor::Black, false, true);
        }

        //레벨 시작과 동시에 로딩 위젯 5초 최상단 유지 후 밝아지는 시퀀스 실행
        ExecuteLoadingSequence([]()
            {
                // 5초 로딩 완료 후 특별히 추가 실행할 콜백 로직은 없음
            }
        );
    }

    //──미니맵 마커 초기화──
    CurrentPlatform = nullptr;
    CurrentTargetPlatform = nullptr;
    CurrentPackage = nullptr;
    bIsTimerActive = false;

    //CurrentTimeLeft = TimeLimit;
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
            CurrentHUD->AddToViewport();//레벨 전환 시 startquest 활성
            // 초기값 설정
            CurrentHUD->UpdateStatus(CurrentStage, /*CurrentWave,*/ DeliveredCount, NumberOfDeliveries);

            //int32 Mins = FMath::FloorToInt(CurrentTimeLeft / 60.0f);
            //int32 Secs = FMath::FloorToInt(CurrentTimeLeft) % 60;
            //CurrentHUD->UpdateTimer(Mins, Secs);
            CurrentHUD->UpdateTimer(0, 0);
        }
    }
    PlayerDrone = Cast<ADrone>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
    // 레벨에 배치된 Spawner 찾기
    Spawner = Cast<AMeteorSpawner>(UGameplayStatics::GetActorOfClass(GetWorld(), AMeteorSpawner::StaticClass()));
    
    //APlayerController* PC = GetWorld()->GetFirstPlayerController();
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

    TArray<AActor*> FoundPlatforms;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADeliveryPlatform::StaticClass(), FoundPlatforms);
    NumberOfDeliveries = FoundPlatforms.Num();
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
    /*AActor* FoundSpawner = UGameplayStatics::GetActorOfClass(GetWorld(), APackageSpawner::StaticClass());
    if (APackageSpawner* PSpawner = Cast<APackageSpawner>(FoundSpawner))
    {
        PSpawner->SpawnPackage();
    }*/

    //스폰된 패키지를 즉시 미니맵 변수에 할당
    TArray<AActor*> FoundPackages;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Package"), FoundPackages);
    if (FoundPackages.Num() > 0)
    {
        CurrentPackage = Cast<ADeliveryPackage>(FoundPackages[0]);
    }

    //CurrentTargetPlatform = GetRandomAvailablePlatform();
    CurrentPlatform = CurrentTargetPlatform; // 미니맵용 변수 재설정

    /*
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
    }*/

    // ── 미니맵 마커 관련 기능 ──
    CurrentPlatform = CurrentTargetPlatform;

    if (CurrentTargetPlatform)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest Started! Target Platform: %s"), *CurrentTargetPlatform->GetName());
    }

    bIsTimerActive = true;
    AApocalypseGameStateBase* GS = GetGameState<AApocalypseGameStateBase>();
    if (IsValid(GS)) {
        GS->SetPlaying();
    }
}

void AApocalypseGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    /*
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
    */
    /*if (bIsTimerActive && CurrentTimeLeft > 0)
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
    }*/

    
    if (PlayerDrone && CurrentHUD)
    {
        //float BatteryPct = PlayerDrone->CurrentBattery / PlayerDrone->MaxBattery;

        float SpeedValue = PlayerDrone->GetCurrentVelocity().Size();

        CurrentHUD->UpdateStats(1, SpeedValue);

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

    // ── 매 프레임마다 미니맵 갱신 ──
    if (bIsTimerActive && CurrentHUD && PlayerDrone)
    {
        // 드론 위치
        FVector DronePos = PlayerDrone->GetActorLocation();

        // 목적지 정보 (StartQuest에서 이미 정해진 CurrentPlatform만 사용)
        bool bHasTarget = IsValid(CurrentPlatform);
        FVector TargetPos = bHasTarget ? CurrentPlatform->GetActorLocation() : FVector::ZeroVector;

        // 화물 정보 갱신
        bool bHasPackage = false;
        FVector PackagePos = FVector::ZeroVector;

        if (!CurrentPackage)
        {
            TArray<AActor*> FoundPackages;
            UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Package"), FoundPackages);
            if (FoundPackages.Num() > 0)
            {
                CurrentPackage = Cast<ADeliveryPackage>(FoundPackages[0]);
            }
        }

        if (CurrentPackage && PlayerDrone->GetAttachedPackage() == nullptr)
        {
            bHasPackage = true;
            PackagePos = CurrentPackage->GetActorLocation();
        }

        // HUD 업데이트 호출
        CurrentHUD->UpdateMinimap(DronePos, bHasTarget, TargetPos, bHasPackage, PackagePos);
    }
    else if (!bIsTimerActive && CurrentHUD)
    {
        // 퀘스트 시작 전에는 마커 숨김.
        CurrentHUD->UpdateMinimap(FVector::ZeroVector, false, FVector::ZeroVector, false, FVector::ZeroVector);
    }
}

// ── 미니맵 데이터 수집 및 전달 ──
void AApocalypseGameMode::UpdateMinimapMarkers()
{
    if (!CurrentHUD || !PlayerDrone) return;

    FVector DronePos = PlayerDrone->GetActorLocation();

    //목표 플랫폼
    bool    bHasTarget = (CurrentTargetPlatform != nullptr);
    FVector TargetPos = bHasTarget ? CurrentTargetPlatform->GetActorLocation() : FVector::ZeroVector;

    //드론에 부착되지 않은 경우에만 마커 표시
    bool    bHasPackage = false;
    FVector PackagePos = FVector::ZeroVector;

    AActor* Attached = PlayerDrone->GetAttachedPackage();
    if (!Attached)
    {
        //씬에 있는 Package 탐색
        TArray<AActor*> FoundPackages;
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Package"), FoundPackages);

        if (FoundPackages.Num() > 0)
        {
            bHasPackage = true;
            PackagePos = FoundPackages[0]->GetActorLocation(); //첫 번째 화물
        }
    }

    //HUD → MinimapWidget 전달
    CurrentHUD->UpdateMinimap(
        DronePos,
        bHasTarget, TargetPos,
        bHasPackage, PackagePos
    );
}

void AApocalypseGameMode::OnPackageDelivered(ADeliveryPlatform* TargetPlatform)
{
    //bIsTimerActive = false;
    //기록 저장 코드
    AApocalypseGameStateBase* GS = GetGameState<AApocalypseGameStateBase>();
    if (IsValid(GS))
    {
        GS->SetNotPlaying();
    }
    //------------
    DeliveredCount++;
    //CurrentWave++;

    // 다음 StartQuest 전까지 미니맵 마커를 비워둔다.
    CurrentPackage = nullptr;
    CurrentPlatform = nullptr;
    CurrentTargetPlatform = nullptr;
    // ────────────────

    if (TargetPlatform)
    {
        TargetPlatform->MarkAsUsed();
    }
    
    if (DeliveredCount >= NumberOfDeliveries)
    {
        CurrentStage++;
        if (CurrentStage > 3) // 모든 스테이지 클리어 시
        {
            bIsTimerActive = false;
            EndGame(true);
        }
        //CurrentWave = 1;
        //UpdateDifficulty();
        if (CurrentHUD)
        {
            // HUD에 스테이지 클리어 UI 호출
            CurrentHUD->ShowStageClearUI();
        }
        FTimerHandle LevelTransitionTimer;
        GetWorldTimerManager().SetTimer(LevelTransitionTimer, this, &AApocalypseGameMode::MoveToNextLevel, 3.0f, false);

        //기존 레벨이 사라지기 직전에 마지막으로 0.5초간 서서히 검은 화면 전환
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC && PC->PlayerCameraManager)
        {
            PC->PlayerCameraManager->StartCameraFade(0.0f, 1.0f, 0.5f, FLinearColor::Black, false, true);
        }

        // 0.5초 대기(페이드 아웃 완료) 직후 바로 다음 레벨 호출
        FTimerHandle DelayTimer;
        GetWorldTimerManager().SetTimer(DelayTimer, [this]()
            {
                this->MoveToNextLevel();
            },
            0.5f, false);

        return;
    }
    /*if (PlayerDrone)
    {
        PlayerDrone->AddBattery(PlayerDrone->MaxBattery);
    }*/
    /*if (CurrentStage > 3) // 모든 스테이지 클리어 시
    {
        bIsTimerActive = false;
        EndGame(true);
    }*/
    /*
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
    }*/

    if(CurrentHUD)
    {
        CurrentHUD->ShowDeliverySuccessUI();
        FString StageInfo = FString::Printf(TEXT("%d - %d"), CurrentStage, 99);
        CurrentHUD->UpdateStageText(StageInfo);
        CurrentHUD->UpdateStats(1.0f, 0.0f);
        CurrentHUD->UpdateStatus(CurrentStage, /*CurrentWave,*/ DeliveredCount, NumberOfDeliveries);
        StartQuest();
        /*
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
        */
    }
    /*
    CurrentTimeLeft = TimeLimit;
    int32 Mins = FMath::FloorToInt(CurrentTimeLeft / 60.0f);
    int32 Secs = FMath::FloorToInt(CurrentTimeLeft) % 60;
    CurrentHUD->UpdateTimer(Mins, Secs);*/
}

void AApocalypseGameMode::UpdateDifficulty()
{
    if (WaveSettings.IsValidIndex(CurrentWaveIndex))
    {
        const FWaveData& CurrentWaveData = WaveSettings[CurrentWaveIndex];

        // 시간 제한 적용
        //TimeLimit = CurrentWaveData.TimeLimit;
        //CurrentTimeLeft = TimeLimit;

        // 목표 배달 수 적용
        TargetDeliveries = CurrentWaveData.TargetDeliveries;

        /*
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
        }*/

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
    //기록 저장 코드
    AApocalypseGameStateBase* GS = GetGameState<AApocalypseGameStateBase>();
    if (IsValid(GS))
    {
        GS->SetNotPlaying();
    }
    //------------

    // UGameplayStatics::SetGamePaused는 결과창이 로딩 뒤에 깔릴 때 실행하도록 람다로 이동
    if (BGMComponent)
    {
        BGMComponent->FadeOut(1.0f, 0.0f);
    }

    //0.5초간 서서히 검은 화면으로 전환
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC && PC->PlayerCameraManager)
    {
        PC->PlayerCameraManager->StartCameraFade(0.0f, 1.0f, 0.5f, FLinearColor::Black, false, true);
    }

    //0.5초 뒤 로딩 시퀀스 시작
    FTimerHandle ResultTimer;
    GetWorldTimerManager().SetTimer(ResultTimer, [this, bIsVictory, PC]()
        {
            //로딩 위젯을 띄우기 직전, 로딩 위젯 바로 아래(Z-Order:0)에 결과 위젯을 먼저 생성해둠
            TSubclassOf<UUserWidget> WidgetToCreate = bIsVictory ? SuccessWidgetClass : FailureWidgetClass;
            if (WidgetToCreate)
            {
                UUserWidget* ResultWidget = CreateWidget<UUserWidget>(GetWorld(), WidgetToCreate);
                if (ResultWidget)
                {
                    ResultWidget->AddToViewport(0);
                }
            }

            //직후 강제로 로딩 위젯을 5초간 최상단에 덮어씌움
            this->ExecuteLoadingSequence([this, PC]()
                {
                    //5초 로딩 유지 시간이 끝나고 화면이 밝아질 때 게임 일시정지 및 조작 설정
                    UGameplayStatics::SetGamePaused(GetWorld(), true);
                    if (PC)
                    {
                        PC->bShowMouseCursor = true;
                        FInputModeUIOnly InputMode;
                        PC->SetInputMode(InputMode);
                    }
                }
            );
        },
        0.5f, false);
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

    if (UApocalypseGameInstance* GI = Cast<UApocalypseGameInstance>(GetGameInstance()))
    {
        GI->ShowLoadingScreen();
    }

    // 맵 이동 실행
    UGameplayStatics::OpenLevel(GetWorld(), NextLevelName);
}

/*
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
}*/

// 로딩 시퀀스 구현 (Fade Out -> 5초 대기 -> Logic 실행 -> Fade In)
void AApocalypseGameMode::ExecuteLoadingSequence(TFunction<void()> LogicAfterLoading)
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    UApocalypseGameInstance* GI = Cast<UApocalypseGameInstance>(GetGameInstance());

    if (!PC || !GI) return;

    //로딩 위젯을 5초간 최상단에 덮어씌움
    GI->ShowLoadingScreen();

    //5초 대기 후 로직 실행
    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(TimerHandle, [this, GI, PC, LogicAfterLoading]()
        {
            //전달받은 핵심 로직 실행
            if (LogicAfterLoading) LogicAfterLoading();

            //5초 끝나면 로딩 화면 사라짐
            GI->HideLoadingScreen();

            //5초 끝나면 동시에 3초간 서서히 화면 밝기 복구
            if (PC->PlayerCameraManager)
            {
                PC->PlayerCameraManager->StartCameraFade(1.0f, 0.0f, 3.0f, FLinearColor::Black, false, false);
            }
        },
        5.0f, false);
}