//ApocalypseGameStateBase.cpp
#include "ApocalypseGameStateBase.h"
#include "ApocalypseGameMode.h"
#include "ApocalypseGameInstance.h"
#include "ApocalypseHUD.h"

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
			GetWorld()->GetAuthGameMode<AApocalypseGameMode>()->CurrentHUD->UpdateTimer((int32)TimeElapsed/60, (int32)TimeElapsed % 60);
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
	if(IsValid(GI)) GI->UpdateRecord(0, TimeElapsed); //need to change to currentlevel
}
float AApocalypseGameStateBase::GetTimeElapsed() {
	return TimeElapsed;
}