#pragma once

#include "CoreMinimal.h"
#include "BaseItem.h"
#include "ReverseControlItem.generated.h"

UCLASS()
class APOCALYPSEDELIVERY_API AReverseControlItem : public ABaseItem
{
	GENERATED_BODY()
	
public:
	AReverseControlItem();

protected:
	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* ItemMesh;
	virtual void ApplyEffect(class ADrone* Drone) override;
};
