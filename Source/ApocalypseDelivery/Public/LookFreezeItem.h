#pragma once

#include "CoreMinimal.h"
#include "BaseItem.h"
#include "LookFreezeItem.generated.h"

UCLASS()
class APOCALYPSEDELIVERY_API ALookFreezeItem : public ABaseItem
{
	GENERATED_BODY()
public:
	ALookFreezeItem();

protected:
	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* ItemMesh;
	virtual void ApplyEffect(class ADrone* Drone) override;
};
