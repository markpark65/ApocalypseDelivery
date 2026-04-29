// Fill out your copyright notice in the Description page of Project Settings.


#include "ChasingEnemy.h"
#include "Drone.h"
#include "ApocalypseDroneController.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AChasingEnemy::AChasingEnemy()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	RootComponent = SphereComp;
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MovementComp = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComp"));
	MovementComp->SetUpdatedComponent(SphereComp);

	IsRepulsive = true;
	
}

// Called when the game starts or when spawned
void AChasingEnemy::BeginPlay()
{
	Super::BeginPlay();
	BasePosition = GetActorLocation();
	IsChasing = false;
	TargetPlayer = nullptr;

	GetWorld()->GetTimerManager().SetTimer(DetectionTimer, this, &AChasingEnemy::CheckTargetCondition, DetectionInterval, true);
	
	if (IsValid(WarningSound)) {
		UE_LOG(LogTemp, Warning, TEXT("Attaching Warning sound!"));
		WarningAudioComp = UGameplayStatics::SpawnSoundAttached(WarningSound, RootComponent);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Warning sound not valid!"));
	}
}

// Called every frame
void AChasingEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (IsChasing)
	{
		AActor* MovementTarget = Cast<ADrone>(TargetPlayer);

		if (!IsValid(MovementTarget)) return;

		//DrawDebugLine(GetWorld(), GetActorLocation(), MovementTarget->GetActorLocation(), FColor::Red, false, 0.017);
		Charge();
	}
	else
	{
		ReturnBase();
	}

	//Rotate to movement direction while moving
	if (GetVelocity().Length() > RotationThreshold) {
		FRotator TargetRotation = UKismetMathLibrary::MakeRotFromX(GetVelocity());
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, RotationInterpRate));
	}
}

// Called to bind functionality to input
void AChasingEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AChasingEnemy::Charge() {
	if (!MovementComp) return;
	//DrawDebugLine(GetWorld(), GetActorLocation(), TargetPlayer->GetActorLocation(), FColor::Red, false, 0.017);
	FVector NewMovement = (TargetPlayer->GetActorLocation() - GetActorLocation()).GetSafeNormal();// * MovementSpeed;
	MovementComp->AddInputVector(NewMovement);

}

void AChasingEnemy::ReturnBase()
{
	if (!MovementComp) return;
	if (FVector::Dist(GetActorLocation(), BasePosition) < ArrivalThreshold) {
		MovementComp->StopMovementImmediately();
		return;
	}
	FVector Direction = (BasePosition - GetActorLocation());

	MovementComp->AddInputVector(Direction);
}

void AChasingEnemy::CheckTargetCondition()
{
	TArray<AActor*> OverlappedActors;
	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), BasePosition, SearchRange, TArray<TEnumAsByte<EObjectTypeQuery>>(), ADrone::StaticClass(), TArray<AActor*>{ this }, OverlappedActors);
	//DrawDebugSphere(GetWorld(), BasePosition, SearchRange, 32, FColor::Cyan, false, DetectionInterval);
	for (AActor* Actor : OverlappedActors) {
		ADrone* Drone = Cast<ADrone>(Actor);
		if (!IsValid(Drone)) {
			continue;
		}
		FHitResult Hit;
		FCollisionQueryParams TraceParams;
		TraceParams.AddIgnoredActor(this);

		//DrawDebugLine(GetWorld(), GetActorLocation(), Drone->GetActorLocation(), FColor::Red, false, DetectionInterval);
		bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, GetActorLocation(),Drone->GetActorLocation(),ECC_Pawn, TraceParams);
		if (bHit && Hit.GetActor() == Drone) {
			TargetPlayer = Drone;
			IsChasing = true;
			return;
		}
	}

	//Failed to Find
	TargetPlayer = nullptr;
	IsChasing = false;
}

void AChasingEnemy::SetBasePosition()
{
	BasePosition = GetActorLocation();
}

void AChasingEnemy::PlayOverlapEffects() {
		if (OverlapParticle) UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), OverlapParticle, GetActorLocation());
		if (OverlapSound) UGameplayStatics::PlaySoundAtLocation(this, OverlapSound, GetActorLocation());
}

void AChasingEnemy::ApplyEffect_Implementation(class ADrone* Drone) {
	if (IsValid(WarningAudioComp)) {
		WarningAudioComp->Stop();
	}
	if (IsRepulsive) {
		Drone->ApplyImpulseVelocity(GetVelocity());
		AApocalypseDroneController* PC = Cast<AApocalypseDroneController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
		if (IsValid(PC)) {
			PC->ShakeCamera(GetVelocity().Length());
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("No Controller!"));
		}
	}
	PlayOverlapEffects();
	Destroy();
}