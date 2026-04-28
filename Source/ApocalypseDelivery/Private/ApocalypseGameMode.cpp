#include "ApocalypseGameMode.h"
#include "ApocalypseGameStateBase.h"
#include "Drone.h"
#include "DeliveryPackage.h"
#include "DeliveryPlatform.h"
#include "ApocalypseHUD.h"



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

    //상자 배달지점 갯수 확인
    TArray<AActor*> FoundPlatforms;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADeliveryPlatform::StaticClass(), FoundPlatforms);
    NumberOfDeliveries = FoundPlatforms.Num();
    
    //레벨별 아이템 필터링
    UpdateDifficulty();
}

void AApocalypseGameMode::StartQuest()
{
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

        //상자 픽업 로직 삭제로 임시 주석처리
        /*
        if (CurrentPackage && PlayerDrone->GetAttachedPackage() == nullptr)
        {
            bHasPackage = true;
            PackagePos = CurrentPackage->GetActorLocation();
        }*/

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

    //패키지 픽업 로직 삭제로 임시 주석처리
    /*
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
    }*/

    //HUD → MinimapWidget 전달
    CurrentHUD->UpdateMinimap(
        DronePos,
        bHasTarget, TargetPos,
        bHasPackage, PackagePos
    );
}

void AApocalypseGameMode::OnPackageDelivered(ADeliveryPlatform* TargetPlatform)
{
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
        if (CurrentHUD)
        {
            // HUD에 스테이지 클리어 UI 호출
            CurrentHUD->ShowStageClearUI();
        }
        FTimerHandle LevelTransitionTimer;
        GetWorldTimerManager().SetTimer(LevelTransitionTimer, this, &AApocalypseGameMode::MoveToNextLevel, 3.0f, false);

        return;
    }

    if(CurrentHUD)
    {
        CurrentHUD->ShowDeliverySuccessUI();
        FString StageInfo = FString::Printf(TEXT("%d - %d"), CurrentStage, 99);
        CurrentHUD->UpdateStageText(StageInfo);
        CurrentHUD->UpdateStats(1.0f, 0.0f);
        CurrentHUD->UpdateStatus(CurrentStage, DeliveredCount, NumberOfDeliveries);
        StartQuest();

    }

}

void AApocalypseGameMode::UpdateDifficulty()
{
    if (CurrentStage == 0) {
        return;
    }
}

void AApocalypseGameMode::GameOver()
{
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

    if (CurrentStage <= 3)
    {
        NextLevelName = FName("MapDraft_EJ");
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
