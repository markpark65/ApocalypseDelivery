#include "ApocalypseHUD.h"
#include "ApocalypseGameMode.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UApocalypseHUD::NativeConstruct()
{
    Super::NativeConstruct();
    if (StartQuestButton)
    {
        StartQuestButton->OnClicked.AddDynamic(this, &UApocalypseHUD::OnStartQuestClicked);
    }
}

void UApocalypseHUD::OnStartQuestClicked()
{
    if (auto* GM = Cast<AApocalypseGameMode>(GetWorld()->GetAuthGameMode()))
    {
        GM->StartQuest();

        this->SetVisibility(ESlateVisibility::HitTestInvisible); // HUD는 보이되 클릭은 안 되게
        StartQuestButton->SetVisibility(ESlateVisibility::Collapsed);

        // 마우스 다시 숨김
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC)
        {
            PC->bShowMouseCursor = false;
            PC->SetInputMode(FInputModeGameOnly());
        }
    }
}
//스테이지 및 웨이브 관리
void UApocalypseHUD::UpdateStatus(int32 Stage, int32 Wave, int32 Count, int32 Target)
{
    if (StageText)
    {
        FString StatusString = FString::Printf(TEXT("STAGE %d - %d"), Stage, Wave);
        StageText->SetText(FText::FromString(StatusString));
    }

    if (DeliveryText)
    {
        DeliveryText->SetText(FText::Format(FText::FromString(TEXT("Delivered: {0} / {1}")), FText::AsNumber(Count), FText::AsNumber(Target)));
    }
}