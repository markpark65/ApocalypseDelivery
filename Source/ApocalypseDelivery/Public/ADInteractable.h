#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ADInteractable.generated.h"

UINTERFACE(MinimalAPI)
class UADInteractable : public UInterface
{
	GENERATED_BODY()
};

class APOCALYPSEDELIVERY_API IADInteractable
{
	GENERATED_BODY()

public:
	virtual void ApplyEffect(class ADrone* Drone) = 0;
	virtual FName GetItemType() const = 0;
};
