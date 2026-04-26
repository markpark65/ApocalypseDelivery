//ApocalypseGameInstance.h

#pragma once

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
        DeliveryRecords.Add(20);
        DeliveryRecords.Add(30);
        DeliveryRecords.Add(40);
        DeliveryRecords.Add(50);
        DeliveryRecords.Add(60);
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

protected:
    UPROPERTY(BlueprintReadWrite)
    TArray<float> DeliveryRecords; // 배달 기록 목록
};
