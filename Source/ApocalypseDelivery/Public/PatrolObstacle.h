// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PatrolObstacle.generated.h"

class USplineComponent;
class USphereComponent;
UCLASS()
class APOCALYPSEDELIVERY_API APatrolObstacle : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APatrolObstacle();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Patrol")
	USplineComponent* SplineComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Patrol")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Patrol")
	USphereComponent* WarningBound;

	UPROPERTY(EditAnywhere, Category = "Patrol")
	float MovementSpeed;

	UPROPERTY(EditAnywhere, Category = "Patrol")
	bool IsLoop = true;          // 순환 여부

	//드론 이동 SFX
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* MovementSound;
	UPROPERTY()
	UAudioComponent* MovementAudioComp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* WarningSound;
	UPROPERTY()
	UAudioComponent* WarningAudioComp;

	UFUNCTION()
	void StartWarning(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void EndWarning(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	

	float CurrentDistance;

};
