//ApocalypseGameInstance.h

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ApocalypseGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class APOCALYPSEDELIVERY_API UApocalypseGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:

    UFUNCTION(BlueprintCallable)
    void UpdateRecord(float ElapsedTime)
    {
        DeliveryRecords.Add(ElapsedTime);
        //테스트용 임의 값
        DeliveryRecords.Add(10);
        //--------------------------
        DeliveryRecords.Sort();
        if (DeliveryRecords.Num() > 10) DeliveryRecords.SetNum(10);
    }

    UFUNCTION(BlueprintCallable)
    TArray<float> GetRecord() const
    {
        return DeliveryRecords;
    }

    UFUNCTION(BlueprintCallable)
    int32 GetRecordCount() const
    {
        return DeliveryRecords.Num();
    }

    // 로딩 화면 위젯 클래스
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> LoadingWidgetClass;

    // 로딩 화면 표시 함수
    UFUNCTION(BlueprintCallable, Category = "UI")
    void ShowLoadingScreen();

    // 로딩 화면 제거 함수
    UFUNCTION(BlueprintCallable, Category = "UI")
    void HideLoadingScreen();

protected:
    UPROPERTY(BlueprintReadWrite)
    TArray<float> DeliveryRecords; // 배달 기록 목록

private:
    UPROPERTY()
    class UUserWidget* CurrentLoadingWidget;
};
