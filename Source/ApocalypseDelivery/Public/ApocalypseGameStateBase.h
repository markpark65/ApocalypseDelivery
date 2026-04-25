//ApocalypseGameStateBase.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ApocalypseGameStateBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeUpdated, float, TimeElapsed);

UCLASS()
class APOCALYPSEDELIVERY_API AApocalypseGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	AApocalypseGameStateBase();

	FOnTimeUpdated OnTimeUpdated;

	void SetNotPlaying();
	void SetPlaying();
	float GetTimeElapsed();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	float TimeElapsed;
	float LastBroadcastedTime;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ApocalypseGameState)
	float BroadcastInterval;
	bool IsPlaying;
};
