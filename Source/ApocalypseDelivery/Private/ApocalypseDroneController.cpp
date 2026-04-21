#include "ApocalypseDroneController.h"
#include "EnhancedInputSubsystems.h"

AApocalypseDroneController::AApocalypseDroneController()
	:InputMappingContext(nullptr)
	, MoveAction(nullptr)
	, LookAction(nullptr)
	, UpDownAction(nullptr)
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