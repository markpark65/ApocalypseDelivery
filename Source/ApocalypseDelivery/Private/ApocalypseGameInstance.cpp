//ApocalypseGameInstance.cpp
#include "ApocalypseGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UApocalypseGameInstance::ShowLoadingScreen()
{
    if (LoadingWidgetClass && !CurrentLoadingWidget) // 중복 생성 방지
    {
        CurrentLoadingWidget = CreateWidget<UUserWidget>(this, LoadingWidgetClass);
        if (CurrentLoadingWidget)
        {
            CurrentLoadingWidget->AddToViewport(10000);
        }
    }
}

void UApocalypseGameInstance::HideLoadingScreen()
{
    if (CurrentLoadingWidget)
    {
        CurrentLoadingWidget->RemoveFromParent();
        CurrentLoadingWidget = nullptr;
    }
}

void UApocalypseGameInstance::RestartStage()
{
    //스테이지 클리어 후 재시작
    CurrentStage = FMath::Max(1, CurrentStage - 1);

    //로딩 화면
    ShowLoadingScreen();
    UGameplayStatics::OpenLevel(GetWorld(), FName("MapDraft_EJ"));
}