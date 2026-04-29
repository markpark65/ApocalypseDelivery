#include "ResultGameMode.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void AResultGameMode::BeginPlay()
{
    Super::BeginPlay();

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC && ResultWidgetClass)
    {
        // 위젯 생성 및 출력
        UUserWidget* ResultUI = CreateWidget<UUserWidget>(GetWorld(), ResultWidgetClass);
        if (ResultUI)
        {
            ResultUI->AddToViewport();

            // UI 입력을 위해 마우스 커서 활성화
            PC->bShowMouseCursor = true;
            PC->SetInputMode(FInputModeUIOnly());
        }
    }
}