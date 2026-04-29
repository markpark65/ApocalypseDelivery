#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "ApocalypseHUD.generated.h"

UCLASS()
class APOCALYPSEDELIVERY_API UApocalypseHUD : public UUserWidget
{
	GENERATED_BODY()
	
public:
    //UI
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
    class UTextBlock* StageText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
    class UTextBlock* DeliveryText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
    class UButton* StartQuestButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
    class UTextBlock* SuccessMessage;

    // ── 미니맵 관련 변수──
    //BP_ApocalypseHUD 위젯 계층에 "MinimapWidget" 이름으로 BP_MinimapWidget 인스턴스를 배치하면 자동으로 바인딩.
    UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
    class UMinimapWidget* MinimapWidget;

    // ── 미니맵 관련 함수──
    //Drone::BeginPlay에서 RenderTarget이 생성된 뒤 MinimapWidget에 연결.
    UFUNCTION(BlueprintCallable, Category = "Minimap")
    void InitializeMinimap(UTextureRenderTarget2D* RenderTarget);

    /*
    // ── 정적 약도 미니맵 사용 시 활성화 ──
    UFUNCTION(BlueprintCallable, Category = "Minimap")
    void InitializeMinimapWithTexture(UTexture2D* MinimapTexture);
    */

    // ApocalypseGameMode::Tick에서 마커 위치 갱신.
    void UpdateMinimap(
        FVector DronePos,
        const TArray<FVector>& PlatformPositions
    );

    // 초기화 시 버튼 클릭 이벤트 연결
    virtual void NativeConstruct() override;

    UFUNCTION()
    void OnStartQuestClicked();

    //타이머
    UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
    void UpdateTimer(int32 Minutes, int32 Seconds, int32 Miliseconds);
    
    //스테이지, 배달 개수 체크
    void UpdateStatus(int32 Stage, /*int32 Wave,*/ int32 Count, int32 Target);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void UpdateDistance(float DistanceInMeters);

    //플레이어 상태 체크
    UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
    void UpdateStats(float BatteryPercent, float CurrentSpeed);

    UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
    void SetInteractionPrompt(bool bVisible, const FString& Message);

    UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
    void ShowDeliverySuccessUI();

    UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
    void ShowStageClearUI();

    UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
    void UpdateStageText(const FString& NewStageInfo);
};
