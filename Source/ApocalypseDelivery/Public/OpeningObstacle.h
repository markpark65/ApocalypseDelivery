// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OpeningObstacle.generated.h"

class UBoxComponent;

UCLASS()
class APOCALYPSEDELIVERY_API AOpeningObstacle : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOpeningObstacle();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Obstacle)
	UBoxComponent* BoxComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Obstacle)
	UStaticMeshComponent* Door1;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Obstacle)
	UStaticMeshComponent* Door2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Obstacle)
	bool IsRepeating;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Obstacle)
	float OpeningSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Obstacle)
	float OpeningDistance;

	bool IsOpening;
	float MovedDistance;
	FTimerHandle RepeatTimer;

	void FlipOpenDirection();

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
