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

    // 초기화 시 버튼 클릭 이벤트 연결
    virtual void NativeConstruct() override;

    UFUNCTION()
    void OnStartQuestClicked();
    //현재 배달 지정 장소 만 띄우기 때문에 사용 안됨
    /*UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void ShowWrongDeliveryUI();*/
    //타이머
    UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
    void UpdateTimer(int32 Minutes, int32 Seconds);
    
    //스테이지, 배달 개수 체크
    void UpdateStatus(int32 Stage, int32 Wave, int32 Count, int32 Target);
    
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
