#pragma once

#include "CoreMinimal.h"
#include "BaseItem.h"
#include "SlowingItem.generated.h"

UCLASS()
class APOCALYPSEDELIVERY_API ASlowingItem : public ABaseItem
{
	GENERATED_BODY()
public:
	ASlowingItem();

protected:
	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* ItemMesh;
	virtual void ApplyEffect(class ADrone* Drone) override;
};
