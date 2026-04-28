//ApocalypseGameInstance.cpp
#include "ApocalypseGameInstance.h"

void UApocalypseGameInstance::ShowLoadingScreen()
{
    if (LoadingWidgetClass)
    {
        CurrentLoadingWidget = CreateWidget<UUserWidget>(this, LoadingWidgetClass);
        if (CurrentLoadingWidget)
        {
            CurrentLoadingWidget->AddToViewport(999); // 가장 위에 표시
        }
    }
}