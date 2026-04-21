// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseItem.h"
#include "BatteryRecoverItem.generated.h"

/**
 * 
 */
UCLASS()
class APOCALYPSEDELIVERY_API ABatteryRecoverItem : public ABaseItem
{
	GENERATED_BODY()
public:
	ABatteryRecoverItem();

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ItemMesh;
	virtual void ApplyEffect(class ADrone* Drone) override;
};
