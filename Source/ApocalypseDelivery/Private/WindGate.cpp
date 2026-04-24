// Fill out your copyright notice in the Description page of Project Settings.


#include "WindGate.h"
#include "Drone.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/FloatingPawnMovement.h"

// Sets default values
AWindGate::AWindGate()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCollisionProfileName("NoCollision");
	RootComponent = MeshComp;

	AccelerationVolume = CreateDefaultSubobject<UCapsuleComponent>(TEXT("AccelerationVolume"));
	AccelerationVolume->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AWindGate::BeginPlay()
{
	Super::BeginPlay();
	AccelerationVolume->OnComponentBeginOverlap.AddDynamic(this, &AWindGate::BeginAccelerate);
	AccelerationVolume->OnComponentEndOverlap.AddDynamic(this, &AWindGate::EndAccelerate);
}

void AWindGate::BeginAccelerate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor->IsA(ADrone::StaticClass()) && OtherComp->IsA(UBoxComponent::StaticClass())) {
		Target = Cast<ADrone>(OtherActor);
        UE_LOG(LogTemp, Warning, TEXT("Accelerating! - %s"), *(Target->GetName()));
    }
}

void AWindGate::EndAccelerate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor->IsA(ADrone::StaticClass()) && OtherComp->IsA(UBoxComponent::StaticClass())) {
		Target = nullptr;
        UE_LOG(LogTemp, Warning, TEXT("Stop Accerler!- %s"), *(OtherActor->GetName()));
    }
}

// Called every frame
void AWindGate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (IsValid(Target)) {
		UFloatingPawnMovement* Movement = Cast<UFloatingPawnMovement>(
			Target->GetMovementComponent());
		if (!Movement) return;

		// MaxSpeed 조정 + velocity 방향 추가
		Target->SetTemporarySpeed(AccelerationRatio, ChangeDuration);
		Movement->Velocity += GetActorForwardVector() * ImpulseSpeed * DeltaTime;
	}
}

