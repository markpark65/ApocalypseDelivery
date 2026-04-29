#include "ApocalypseGameMode.h"
#include "ApocalypseGameStateBase.h"
#include "ApocalypseGameInstance.h"
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

    GI = GetGameInstance<UApocalypseGameInstance>();
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
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

    //상자 배달지점 갯수 확인
    TArray<AActor*> FoundPlatforms;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADeliveryPlatform::StaticClass(), FoundPlatforms);
    NumberOfDeliveries = FoundPlatforms.Num();

    for (AActor* Actor : FoundPlatforms)
    {
        if (ADeliveryPlatform* Platform = Cast<ADeliveryPlatform>(Actor))
        {
            AllPlatforms.Add(Platform);
        }
    }

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
            CurrentHUD->UpdateStatus(GI->CurrentStage, /*CurrentWave,*/ DeliveredCount, NumberOfDeliveries);

            //int32 Mins = FMath::FloorToInt(CurrentTimeLeft / 60.0f);
            //int32 Secs = FMath::FloorToInt(CurrentTimeLeft) % 60;
            //CurrentHUD->UpdateTimer(Mins, Secs);
            CurrentHUD->UpdateTimer(0, 0, 0);
        }
    }
    PlayerDrone = Cast<ADrone>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));

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
    
    //스테이지별 아이템 필터링
    UpdateDifficulty();
}

void AApocalypseGameMode::StartQuest()
{
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
        /*
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
        */
    }

    // ── 매 프레임마다 미니맵 갱신 ──
    if (bIsTimerActive && CurrentHUD && PlayerDrone)
    {
        // 드론 위치
        FVector DronePos = PlayerDrone->GetActorLocation();
        // 목적지 정보
        TArray<FVector> PlatformPositions;
        for (ADeliveryPlatform* Platform : AllPlatforms)
        {
            if (Platform && !Platform->bIsUsed)
            {
                PlatformPositions.Add(Platform->GetActorLocation());
            }
        }
        // HUD 업데이트 호출
        CurrentHUD->UpdateMinimap(DronePos, PlatformPositions/*, bHasPackage, PackagePos*/);
    }
    else if (!bIsTimerActive && CurrentHUD)
    {
        // 퀘스트 시작 전에는 마커 숨김.
        CurrentHUD->UpdateMinimap(FVector::ZeroVector, TArray<FVector>()/*, false, FVector::ZeroVector*/);
    }
}

// ── 미니맵 데이터 수집 및 전달 ──
void AApocalypseGameMode::UpdateMinimapMarkers()
{
    if (!CurrentHUD || !PlayerDrone) return;

    FVector DronePos = PlayerDrone->GetActorLocation();

    //목표 플랫폼
    TArray<FVector> PlatformPositions;
    for (ADeliveryPlatform* Platform : AllPlatforms)
    {
        if (Platform && !Platform->bIsUsed)
        {
            PlatformPositions.Add(Platform->GetActorLocation());
        }
    }

    //HUD → MinimapWidget 전달
    CurrentHUD->UpdateMinimap(
        DronePos,
        PlatformPositions/*,
        bHasPackage, PackagePos*/
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
    //CurrentPackage = nullptr;
    //CurrentPlatform = nullptr;
    //CurrentTargetPlatform = nullptr;
    // ────────────────
    
    /*MarkAsUsed() ADeliveryPlatform에서 직접 호출하도록 수정 완료
    if (TargetPlatform)
    {
        TargetPlatform->MarkAsUsed();
    }*/

    
    if (DeliveredCount >= NumberOfDeliveries)
    {
        GI->CurrentStage++;
        if (GI->CurrentStage > 3) // 모든 스테이지 클리어 시
        {
            bIsTimerActive = false;
            EndGame(/*true*/);
            return;
        }
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

    if(CurrentHUD)
    {
        CurrentHUD->ShowDeliverySuccessUI();
        FString StageInfo = FString::Printf(TEXT("%d - %d"), GI->CurrentStage, 99); //뒤에 숫자 지금 안쓰임
        CurrentHUD->UpdateStageText(StageInfo);
        CurrentHUD->UpdateStats(1.0f, 0.0f);
        CurrentHUD->UpdateStatus(GI->CurrentStage, DeliveredCount, NumberOfDeliveries);
        StartQuest();

    }

}

void AApocalypseGameMode::UpdateDifficulty()
{
    UE_LOG(LogTemp, Warning, TEXT("Setting Stage %d!"), GI->CurrentStage);
    if (GI->CurrentStage < 2) {
        TArray<AActor*> IntermediateActors;
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Intermediate"), IntermediateActors);
        UE_LOG(LogTemp, Warning, TEXT("Destroying %d Intermediate actors!"), IntermediateActors.Num());
        for (AActor* DisableTarget : IntermediateActors) {
            DisableTarget->Destroy();
        }
    }
    if (GI->CurrentStage < 3) {
        TArray<AActor*> ProActors;
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Pro"), ProActors);
        UE_LOG(LogTemp, Warning, TEXT("Destroying %d Pro actors!"), ProActors.Num());
        for (AActor* DisableTarget : ProActors) {
            DisableTarget->Destroy();
        }
    }

}

void AApocalypseGameMode::GameOver()
{
    // UI에 게임오버 알림
    UGameplayStatics::SetGamePaused(GetWorld(), true);
}

void AApocalypseGameMode::EndGame(/*bool bIsVictory*/)
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
    GetWorldTimerManager().SetTimer(ResultTimer, [this, /*bIsVictory,*/ PC]()
        {
            //로딩 위젯을 띄우기 직전, 로딩 위젯 바로 아래(Z-Order:0)에 결과 위젯을 먼저 생성해둠
            TSubclassOf<UUserWidget> WidgetToCreate = /*bIsVictory ? */SuccessWidgetClass/* : FailureWidgetClass*/;
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

    if (GI->CurrentStage <= 3)
    {
        NextLevelName = FName("MapDraft_EJ");
    }
    else
    {
        NextLevelName = FName("MainMenu"); // 혹은 최종 승리 화면
    }

    UE_LOG(LogTemp, Warning, TEXT("Moving to Level: %s"), *NextLevelName.ToString());

    if (IsValid(GI))
    {
        GI->ShowLoadingScreen();
    }

    // 맵 이동 실행
    UGameplayStatics::OpenLevel(GetWorld(), NextLevelName);
}

// 로딩 시퀀스 구현 (Fade Out -> 5초 대기 -> Logic 실행 -> Fade In)
void AApocalypseGameMode::ExecuteLoadingSequence(TFunction<void()> LogicAfterLoading)
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    //UApocalypseGameInstance* GI = Cast<UApocalypseGameInstance>(GetGameInstance()); 클래스 맴버 변수로 변경

    if (!PC || !GI) return;

    //로딩 위젯을 5초간 최상단에 덮어씌움
    GI->ShowLoadingScreen();

    //5초 대기 후 로직 실행
    FTimerHandle TimerHandle;
    UApocalypseGameInstance* TempGI = GI;
    GetWorldTimerManager().SetTimer(TimerHandle, [/*this, GI*/ TempGI, PC, LogicAfterLoading]()
        {
            //전달받은 핵심 로직 실행
            if (LogicAfterLoading) LogicAfterLoading();

            //5초 끝나면 로딩 화면 사라짐
            TempGI->HideLoadingScreen();

            //5초 끝나면 동시에 3초간 서서히 화면 밝기 복구
            if (IsValid(PC) && PC->PlayerCameraManager)
            {
                PC->PlayerCameraManager->StartCameraFade(1.0f, 0.0f, 3.0f, FLinearColor::Black, false, false);
            }
        },
        5.0f, false);
}
