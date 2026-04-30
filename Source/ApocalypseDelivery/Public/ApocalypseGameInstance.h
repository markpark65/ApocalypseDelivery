//ApocalypseGameInstance.h

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "ApocalypseGameMode.h"
#include "ApocalypseHUD.h"
#include "Engine/GameInstance.h"
#include "ApocalypseGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FStageRecord {
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    TArray<float> Records;
};

UCLASS()
class APOCALYPSEDELIVERY_API UApocalypseGameInstance : public UGameInstance
{
    GENERATED_BODY()
public:
    virtual void Init() override
    {
        Super::Init();
        // 3개 스테이지 슬롯 미리 생성
        DeliveryRecords.SetNum(3);
        CurrentStage = 1;
    }

    UFUNCTION(BlueprintCallable)
    void UpdateRecord(int32 Stage, float ElapsedTime)
    {
        int32 StageIndex = Stage - 1;

        LatestRecordIndex = StageIndex;

        if (DeliveryRecords.IsValidIndex(StageIndex))
        {
            DeliveryRecords[StageIndex].Records.Add(ElapsedTime);
            DeliveryRecords[StageIndex].Records.Sort();
            //방금 넣은 기록이 10위 안에 드는지 검사
            int32 MyRank = DeliveryRecords[StageIndex].Records.Find(ElapsedTime);
            bIsNewRecord = (MyRank != INDEX_NONE && MyRank < 10);

            // 10개가 넘어가면 하위 기록 자르기
            if (DeliveryRecords[StageIndex].Records.Num() > 10)
            {
                DeliveryRecords[StageIndex].Records.SetNum(10);
            }
            /*//테스트용 임의 값
            DeliveryRecords[0].Records.Add(10);
            DeliveryRecords[0].Records.Add(20);
            DeliveryRecords[0].Records.Add(30);
            DeliveryRecords[0].Records.Add(40);
            DeliveryRecords[0].Records.Add(50);
            DeliveryRecords[0].Records.Add(60);
            DeliveryRecords[1].Records.Add(50);
            DeliveryRecords[1].Records.Add(60);
            DeliveryRecords[2].Records.Add(50);
            DeliveryRecords[2].Records.Add(60);
            //--------------------------
            DeliveryRecords[0].Records.Sort();
            DeliveryRecords[1].Records.Sort();
            DeliveryRecords[2].Records.Sort();
            if (DeliveryRecords[0].Records.Num() > 10) DeliveryRecords[0].Records.SetNum(10);
            if (DeliveryRecords[1].Records.Num() > 10) DeliveryRecords[1].Records.SetNum(10);
            if (DeliveryRecords[2].Records.Num() > 10) DeliveryRecords[2].Records.SetNum(10);*/
            //if (DeliveryRecords[Stage].Records.Num() > 10) DeliveryRecords[Stage].Records.SetNum(10);
        }
    }

    UFUNCTION(BlueprintCallable)
    TArray<FStageRecord> GetRecord() const
    {
        return DeliveryRecords;
    }

    UFUNCTION(BlueprintCallable)
    int32 GetRecordCount() const
    {
        return DeliveryRecords.Num();
    }

    //결과 화면에서 Restart
    UFUNCTION(BlueprintCallable, Category = "GameLogic")
    void RestartStage();

    // 로딩 화면 위젯 클래스
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> LoadingWidgetClass;

    //게임이 처음 켜졌는지 체크
    UPROPERTY(BlueprintReadWrite, Category = "Loading")
    bool bIsFirstBoot = true;

    // 로딩 화면 표시 함수
    UFUNCTION(BlueprintCallable, Category = "UI")
    void ShowLoadingScreen();

    // 로딩 화면 제거 함수
    UFUNCTION(BlueprintCallable, Category = "UI")
    void HideLoadingScreen();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameRules")
    int32 CurrentStage;

    //결과 창 이후에 이동할 실제 스테이지 이름
    UPROPERTY(BlueprintReadWrite, Category = "GameLogic")
    FName PendingNextLevel;

    //이번 판에 10위 이내 신기록을 달성했는지 확인하는 플래그
    UPROPERTY(BlueprintReadWrite, Category = "GameLogic")
    bool bIsNewRecord = false;

    UPROPERTY(BlueprintReadWrite, Category = "GameLogic")
    int32 LatestRecordIndex = 0;

    //치트용
    UFUNCTION(Exec)
    void IncreaseStageNumber() {
        CurrentStage++;
        UE_LOG(LogTemp, Warning, TEXT("CurrentStage number has been changed to %d"), CurrentStage);
        AApocalypseGameMode* GM = GetWorld()->GetAuthGameMode<AApocalypseGameMode>();
        GM->MoveToNextLevel();
    }
protected:
    UPROPERTY(BlueprintReadWrite)
    TArray<FStageRecord> DeliveryRecords; // 배달 기록 목록

private:
    UPROPERTY()
    class UUserWidget* CurrentLoadingWidget;
};