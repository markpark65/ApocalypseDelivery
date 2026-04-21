#pragma once

#include "CoreMinimal.h"
#include "BaseItem.h"
#include "CureItem.generated.h"

UCLASS()
class APOCALYPSEDELIVERY_API ACureItem : public ABaseItem
{
	GENERATED_BODY()
public:
	ACureItem();

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ItemMesh;
	virtual void ApplyEffect(class ADrone* Drone) override;
};
