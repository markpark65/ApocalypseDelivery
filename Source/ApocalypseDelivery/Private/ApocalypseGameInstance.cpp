//ApocalypseGameInstance.cpp
#include "ApocalypseGameInstance.h"

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