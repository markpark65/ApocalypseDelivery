#pragma once

#include "CoreMinimal.h"
#include "BaseItem.h"
#include "ShieldItem.generated.h"

UCLASS()
class APOCALYPSEDELIVERY_API AShieldItem : public ABaseItem
{
	GENERATED_BODY()
public:
	AShieldItem();

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ItemMesh;
	virtual void ApplyEffect(class ADrone* Drone) override;
};
