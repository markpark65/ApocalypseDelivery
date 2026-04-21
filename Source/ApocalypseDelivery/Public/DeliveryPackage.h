#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DeliveryPackage.generated.h"

UCLASS()
class APOCALYPSEDELIVERY_API ADeliveryPackage : public AActor
{
	GENERATED_BODY()
	
public:	
	ADeliveryPackage();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UBoxComponent* CollisionComp;

};
