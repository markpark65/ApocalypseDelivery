// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WindGate.generated.h"

class UCapsuleComponent;
class ADrone;
UCLASS()
class APOCALYPSEDELIVERY_API AWindGate : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWindGate();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = WindGate)
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = WindGate)
	UCapsuleComponent* AccelerationVolume;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = WindGate)
	float AccelerationRatio;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = WindGate)
	float ChangeDuration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = WindGate)
	float ImpulseSpeed;

	UFUNCTION()
	void BeginAccelerate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void EndAccelerate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


	ADrone* Target;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
