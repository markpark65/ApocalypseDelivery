#pragma once

#include "CoreMinimal.h"
#include "BaseItem.h"
#include "AirMine.generated.h"

UCLASS()
class APOCALYPSEDELIVERY_API AAirMine : public ABaseItem
{
	GENERATED_BODY()
	
public:
    AAirMine();

protected:

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* ItemMesh;

    virtual void ApplyEffect(class ADrone* Drone) override;
};
