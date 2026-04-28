#include "ApocalypseHUD.h"
#include "ApocalypseGameMode.h"
#include "MinimapWidget.h"
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
void UApocalypseHUD::UpdateStatus(int32 Stage, /*int32 Wave,*/ int32 Count, int32 Target)
{
    if (StageText)
    {
        FString StatusString = FString::Printf(TEXT("STAGE %d"), Stage/*, Wave*/);
        StageText->SetText(FText::FromString(StatusString));
    }

    if (DeliveryText)
    {
        DeliveryText->SetText(FText::Format(FText::FromString(TEXT("Delivered: {0} / {1}")), FText::AsNumber(Count), FText::AsNumber(Target)));
    }
}

// ── 미니맵 관련 함수 구현──

void UApocalypseHUD::InitializeMinimap(UTextureRenderTarget2D* RenderTarget)
{
    if (MinimapWidget && RenderTarget)
    {
        MinimapWidget->SetRenderTarget(RenderTarget);
        //UE_LOG(LogTemp, Log, TEXT("Minimap RenderTarget initialized."));
    }
}

/*
// ── 정적 약도 미니맵 사용 시 추가 ──
void UApocalypseHUD::InitializeMinimapWithTexture(UTexture2D* MinimapTexture)
{
    if (MinimapWidget && MinimapTexture)
    {
        MinimapWidget->SetStaticBackground(MinimapTexture);
    }
}
*/

void UApocalypseHUD::UpdateMinimap(
    FVector DronePos/*,
    bool bHasTarget, FVector TargetPos,
    bool bHasPackage, FVector PackagePos*/)
{
    if (MinimapWidget)
    {
        MinimapWidget->UpdateMarkers(
            DronePos/*,
            bHasTarget, TargetPos,
            bHasPackage, PackagePos*/
        );
    }
}