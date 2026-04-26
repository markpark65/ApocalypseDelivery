#include "ApocalypseDroneController.h"
#include "EnhancedInputSubsystems.h"

AApocalypseDroneController::AApocalypseDroneController()
	:InputMappingContext(nullptr)
	, MoveAction(nullptr)
	, LookAction(nullptr)
	//, UpDownAction(nullptr)
	, RollAction(nullptr)
	, PickupAction(nullptr)
{
	
}

void AApocalypseDroneController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (InputMappingContext)
			{
				Subsystem->AddMappingContext(InputMappingContext, 0);
			}
		}
	}
}


void AApocalypseDroneController::SetCameraFade(float duration)
{
	PlayerCameraManager->StartCameraFade(0, 1, 1, FLinearColor::Black, true, true);
	GetWorld()->GetTimerManager().SetTimer(FadeTimerHandle, this, &AApocalypseDroneController::ResetCameraFade, duration, false);
}
void AApocalypseDroneController::ResetCameraFade()
{
	PlayerCameraManager->StartCameraFade(1, 0,1, FLinearColor::Black, true, false);
}
