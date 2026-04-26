// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PatrolObstacle.generated.h"

class USplineComponent;
UCLASS()
class APOCALYPSEDELIVERY_API APatrolObstacle : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APatrolObstacle();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Patrol")
	USplineComponent* SplineComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Patrol")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere, Category = "Patrol")
	float MovementSpeed;

	UPROPERTY(EditAnywhere, Category = "Patrol")
	bool IsLoop = true;          // 순환 여부

	float CurrentDistance;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
