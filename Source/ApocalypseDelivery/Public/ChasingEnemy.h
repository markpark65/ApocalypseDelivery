// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ChasingEnemy.generated.h"

class USphereComponent;
class UFloatingPawnMovement;
UCLASS()
class APOCALYPSEDELIVERY_API AChasingEnemy : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AChasingEnemy();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crow)
	USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crow)
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crow)
	UFloatingPawnMovement* MovementComp;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Crow)
	float SearchRange;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Crow)
	float ArrivalThreshold;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Crow)
	float DetectionInterval;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Crow)
	float RotationThreshold;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Crow)
	float RotationInterpRate;

	FVector BasePosition;
	AActor* TargetPlayer;
	bool IsChasing;
	FTimerHandle DetectionTimer;

	UFUNCTION(BlueprintCallable, Category = Crow)
	void Charge();

	UFUNCTION(BlueprintCallable, Category = Crow)
	void ReturnBase();

	UFUNCTION(BlueprintCallable, Category = Crow)
	void CheckTargetCondition();

	UFUNCTION(BlueprintCallable, Category = Crow)
	void SetBasePosition();


	UFUNCTION()
	void OnCollision(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

};
