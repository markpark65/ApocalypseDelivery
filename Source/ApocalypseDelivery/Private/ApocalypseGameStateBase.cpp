//ApocalypseGameStateBase.cpp
#include "ApocalypseGameStateBase.h"
#include "ApocalypseGameInstance.h"

AApocalypseGameStateBase::AApocalypseGameStateBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AApocalypseGameStateBase::BeginPlay()
{
	Super::BeginPlay();
	TimeElapsed = 0.0f;
	LastBroadcastedTime = 0.0f;
	IsPlaying = false;
}

void AApocalypseGameStateBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (IsPlaying) {
		TimeElapsed += DeltaTime;
		//UE_LOG(LogTemp, Warning, TEXT("TimeElapsed: %f"), TimeElapsed);

		if (TimeElapsed - LastBroadcastedTime >= BroadcastInterval) {
			OnTimeUpdated.Broadcast(TimeElapsed);
			LastBroadcastedTime = TimeElapsed;
		}
	}
}


void AApocalypseGameStateBase::SetPlaying() {
	IsPlaying = true;
}

void AApocalypseGameStateBase::SetNotPlaying() {
	IsPlaying = false;
	UApocalypseGameInstance * GI = GetGameInstance<UApocalypseGameInstance>();
	if(IsValid(GI)) GI->UpdateRecord(TimeElapsed);
}
float AApocalypseGameStateBase::GetTimeElapsed() {
	return TimeElapsed;
}