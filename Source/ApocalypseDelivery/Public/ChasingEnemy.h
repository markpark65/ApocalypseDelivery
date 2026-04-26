// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ADInteractable.h"

#include "GameFramework/Pawn.h"
#include "ChasingEnemy.generated.h"

class USphereComponent;
class ADrone;
class UFloatingPawnMovement;
UCLASS()
class APOCALYPSEDELIVERY_API AChasingEnemy : public APawn, public IADInteractable
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Crow)
	bool IsRepulsive;

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

	//Collision Effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	class UParticleSystem* OverlapParticle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	class USoundBase* OverlapSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	FString PickupMessage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TSubclassOf<class UUserWidget> ItemMessageWidgetClass;

	void PlayOverlapEffects();

	void ShowPickupUI(ADrone* Drone);

	UFUNCTION(BlueprintNativeEvent)
	void ApplyEffect(ADrone* Drone);
	virtual void ApplyEffect_Implementation(ADrone* Drone);
	virtual FName GetItemType() const {
		return FName("");
	}

};
