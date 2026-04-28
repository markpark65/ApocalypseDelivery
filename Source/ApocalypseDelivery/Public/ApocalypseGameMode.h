#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/DataTable.h"
#include "ApocalypseGameMode.generated.h"

/*
USTRUCT(BlueprintType)
struct FWaveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TargetDeliveries = 3; // 목표 배달 수

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeLimit = 180.0f;    // 제한 시간

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MeteorSpawnInterval = 2.0f; // 운석 속도

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MineSpawnCount = 0;    // 지뢰 개수

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DebuffItemCount = 0;   // 디버프 아이템 개수

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BuffItemCount = 1;
};*/

class ADrone;
class UApocalypseGameInstance;

UCLASS()
class APOCALYPSEDELIVERY_API AApocalypseGameMode : public AGameModeBase
{
	GENERATED_BODY()
	

public:
    AApocalypseGameMode();

    // 스테이지 데이터
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
    int32 DeliveredCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
    int32 TargetDeliveries = 3;

    // 배달 성공 시 호출
    UFUNCTION(BlueprintCallable, Category = "GameLogic")
    void OnPackageDelivered(class ADeliveryPlatform* TargetPlatform);

    // 게임 오버 시 호출
    UFUNCTION(BlueprintCallable, Category = "GameLogic")
    void GameOver();

    // 퀘스트 시작 (UI에서 버튼 클릭 시)
    UFUNCTION(BlueprintCallable, Category = "GameLogic")
    void StartQuest();

    UFUNCTION(BlueprintCallable, Category = "GameLogic")
    void MoveToNextLevel();

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<class UUserWidget> HUDWidgetClass;

    UPROPERTY()
    class UApocalypseHUD* CurrentHUD;

    UFUNCTION(BlueprintCallable)
    void EndGame(bool bIsVictory);

    FORCEINLINE bool IsTimerActive() const { return bIsTimerActive; }


protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    // 성공 시 띄울 위젯 클래스
    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<class UUserWidget> SuccessWidgetClass;

    // 실패 시 띄울 위젯 클래스
    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<class UUserWidget> FailureWidgetClass;

    int32 NumberOfDeliveries;

    UPROPERTY(EditAnywhere, Category = "Sound")
    class USoundBase* BackgroundMusic;

    // 재생 중인 사운드를 관리하기 위한 컴포넌트
    UPROPERTY()
    class UAudioComponent* BGMComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameLogic")
    float TimeLimit = 180.0f;

    //float CurrentTimeLeft;
    bool bIsTimerActive = false;

    //──미니맵 마커 관련 변수──
    // 현재 배달 목표 플랫폼
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameLogic")
    class ADeliveryPlatform* CurrentPlatform;

    // 현재 스폰된 화물 (미니맵 표시용)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameLogic")
    class ADeliveryPackage* CurrentPackage;

    // 공통 로딩 시퀀스 함수
    void ExecuteLoadingSequence(TFunction<void()> LogicAfterLoading);

private:
    void UpdateDifficulty();
    UPROPERTY()
    class ADrone* PlayerDrone;

    UPROPERTY()
    class ADeliveryPlatform* CurrentTargetPlatform;

    //──미니맵 마커 데이터를 수집해 HUD에 전달하는 함수──
    void UpdateMinimapMarkers();

    UApocalypseGameInstance* GI;
};