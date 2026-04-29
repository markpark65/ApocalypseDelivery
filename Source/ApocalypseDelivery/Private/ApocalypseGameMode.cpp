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
        if (GI) GI->HideLoadingScreen();

        if (PC && PC->PlayerCameraManager)
        {
            PC->PlayerCameraManager->StartCameraFade(1.0f, 0.0f, 2.0f, FLinearColor::Black, false, false);
        }
    }

    //상자 배달지점 갯수 확인
    TArray<AActor*> FoundPlatforms;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADeliveryPlatform::StaticClass(), FoundPlatforms);
    for (AActor* Actor : FoundPlatforms)
    {
        if (Actor->ActorHasTag("Pro") && GI->CurrentStage < 3)
        {
            Actor->Destroy(); // 스테이지 3 미만이면 Pro 플랫폼 파괴
            continue;
        }
        if (Actor->ActorHasTag("Intermediate") && GI->CurrentStage < 2)
        {
            Actor->Destroy(); // 스테이지 2 미만이면 Intermediate 플랫폼 파괴
            continue;
        }

        // 현재 있는 플랫폼 배열에 넣고 개수를 셉니다.
        if (ADeliveryPlatform* Platform = Cast<ADeliveryPlatform>(Actor))
        {
            if (!Actor->ActorHasTag("Fake"))
            {
                AllPlatforms.Add(Platform);
            }
            else {
                AllPlatforms.Add(Platform);
            }
        }
    }

    //실제로 배달해야 할 개수만 설정
    NumberOfDeliveries = 0;
    for (auto P : AllPlatforms) {
        if (!P->ActorHasTag("Fake")) NumberOfDeliveries++;
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
{/*
    //기록 저장 코드
    AApocalypseGameStateBase* GS = GetGameState<AApocalypseGameStateBase>();
    if (IsValid(GS))
    {
        GS->SetNotPlaying();
    }*/
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
        bIsTimerActive = false;

        if (GI)
        {
            GI->CurrentStage++;
        }

        //페이드 시작
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC && PC->PlayerCameraManager)
        {
            PC->PlayerCameraManager->StartCameraFade(0.0f, 1.0f, 0.5f, FLinearColor::Black, false, true);
        }

        EndGame();
        return;
    }

    //배달 성공 시 HUD 업데이트
    if (CurrentHUD)
    {
        CurrentHUD->ShowDeliverySuccessUI();
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

void AApocalypseGameMode::EndGame()
{
    //기록 저장 코드
    AApocalypseGameStateBase* GS = GetGameState<AApocalypseGameStateBase>();
    if (IsValid(GS))
    {
        GS->SetNotPlaying();
    }
    //------------

    if (BGMComponent) BGMComponent->FadeOut(1.0f, 0.0f);

    if (GI->CurrentStage <= 3)
    {
        GI->PendingNextLevel = FName("MapDraft_EJ"); // 다음 스테이지로 쓰일 맵
    }
    else
    {
        GI->CurrentStage = 1;
        GI->PendingNextLevel = FName("MainMenu");    // 3스테이지 최종 클리어 시
        UGameplayStatics::OpenLevel(GetWorld(), FName("EndLevel"));
    }

    //로딩 화면 띄우기
    GI->ShowLoadingScreen();

    //어디서든 게임이 끝나면 무조건 결과 레벨로 이동
    UGameplayStatics::OpenLevel(GetWorld(), FName("ResultLevel"));
}

void AApocalypseGameMode::MoveToNextLevel()
{

    if (!GI) return;

    // 1. 다음에 가야 할 실제 레벨 계산
    GI->CurrentStage++;
    FName ActualNextLevel = (GI->CurrentStage <= 3) ? FName("MapDraft_EJ") : FName("MainMenu");

    // 2. GameInstance에 목적지 저장
    GI->PendingNextLevel = ActualNextLevel;

    // 3. 실제 이동은 결과 전용 레벨로 수행
    GI->ShowLoadingScreen();
    UGameplayStatics::OpenLevel(GetWorld(), FName("ResultLevel")); // 결과 레벨 이름

    /* 구 로직
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
    */
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
