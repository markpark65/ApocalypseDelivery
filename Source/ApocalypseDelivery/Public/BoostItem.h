// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseItem.h"
#include "BoostItem.generated.h"

/**
 * 
 */
UCLASS()
class APOCALYPSEDELIVERY_API ABoostItem : public ABaseItem
{
	GENERATED_BODY()
public:
	ABoostItem();

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ItemMesh;
	virtual void ApplyEffect(class ADrone* Drone) override;
};
