//ApocalypseGameStateBase.h
#pragma once

#include "CoreMinimal.h"

#include "GameFramework/GameStateBase.h"
#include "ApocalypseGameStateBase.generated.h"

class AApocalypseHUD;

UCLASS()
class APOCALYPSEDELIVERY_API AApocalypseGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	AApocalypseGameStateBase();

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
