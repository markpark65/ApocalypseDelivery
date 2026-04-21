#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Meteor.generated.h"

UCLASS()
class APOCALYPSEDELIVERY_API AMeteor : public AActor
{
	GENERATED_BODY()
	
public:	
	AMeteor();

	//컴포넌트 선언
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UProjectileMovementComponent* ProjectileMovement;

	void SetMeteorSpeed(float NewSpeed);

protected:
	virtual void BeginPlay() override;

	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	UPROPERTY(EditAnywhere, Category = "Sound")
	class USoundBase* ExplosionSound;
};
