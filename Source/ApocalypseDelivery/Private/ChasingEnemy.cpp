// Fill out your copyright notice in the Description page of Project Settings.


#include "ChasingEnemy.h"
#include "Drone.h"
#include "ApocalypseDroneController.h"

#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
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

	ParticleComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleComp"));
	ParticleComp->SetupAttachment(RootComponent);

	MovementComp = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComp"));
	MovementComp->SetUpdatedComponent(SphereComp);

	RecognitionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("RecognitionSphere"));
	RecognitionSphere->SetAbsolute(true, false, false);
	RecognitionSphere->SetupAttachment(RootComponent);

	WarningAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("WarningAudioComp"));
	WarningAudioComp->SetupAttachment(RootComponent);
	WarningAudioComp->bAutoActivate = false;
	IsRepulsive = true;
	
}

// Called when the game starts or when spawned
void AChasingEnemy::BeginPlay()
{
	Super::BeginPlay();
	IsChasing = false;
	TargetPlayer = nullptr;

	if (IsValid(WarningAudioComp->Sound))
	{
		WarningAudioComp->Play();
	}
	RecognitionSphere->SetWorldLocation(GetActorLocation());
	//DrawDebugSphere(GetWorld(), RecognitionSphere->GetComponentLocation(), RecognitionSphere->GetUnscaledSphereRadius(), 32, FColor::Cyan, false, 3600);
	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AChasingEnemy::OnPlayerCollision);
	RecognitionSphere->OnComponentBeginOverlap.AddDynamic(this, &AChasingEnemy::OnRecogRangeEntered);
	RecognitionSphere->OnComponentEndOverlap.AddDynamic(this, &AChasingEnemy::OnRecogRangeExit);
}

// Called every frame
void AChasingEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (IsChasing)
	{
		AActor* MovementTarget = Cast<ADrone>(TargetPlayer);

		if (!IsValid(MovementTarget)) return;
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
	FVector NewMovement = (TargetPlayer->GetActorLocation() - GetActorLocation()).GetSafeNormal();// * MovementSpeed;
	MovementComp->AddInputVector(NewMovement);

}

void AChasingEnemy::ReturnBase()
{
	if (!MovementComp) return;
	if (FVector::Dist(GetActorLocation(), RecognitionSphere->GetComponentLocation()) < ArrivalThreshold) {
		MovementComp->StopMovementImmediately();
		return;
	}
	FVector Direction = (RecognitionSphere->GetComponentLocation() - GetActorLocation());

	MovementComp->AddInputVector(Direction);
}

void AChasingEnemy::CheckTargetCondition()
{
	UE_LOG(LogTemp, Warning, TEXT("Checking Target!"));
	if (!IsValid(TargetPlayer)) {
		return;
	}
	FHitResult Hit;
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(this);
	//DrawDebugLine(GetWorld(), GetActorLocation(), TargetPlayer->GetActorLocation(), FColor::Red, false, DetectionInterval);
	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, GetActorLocation(), TargetPlayer->GetActorLocation(),ECC_Pawn, TraceParams);
	if (bHit && Hit.GetActor() == TargetPlayer) {
		IsChasing = true;
		return;
	}

	IsChasing = false;
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
	if (IsValid(WarningAudioComp) && WarningAudioComp->IsPlaying()) WarningAudioComp->Stop();
	Destroy();
}


void AChasingEnemy::OnPlayerCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ADrone* PlayerDrone = Cast<ADrone>(OtherActor);
	if (IsValid(PlayerDrone) && IsValid(OtherComp) && OtherComp->IsA(USphereComponent::StaticClass())) {
		UE_LOG(LogTemp, Warning, TEXT("Colliding!"));
		ApplyEffect(PlayerDrone);
	}
}

void AChasingEnemy::OnRecogRangeEntered(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if (IsValid(OtherActor) && OtherActor->IsA(ADrone::StaticClass()) && IsValid(OtherComp) && OtherComp->IsA(USphereComponent::StaticClass())) {
		UE_LOG(LogTemp, Warning, TEXT("Entered Search Area!"));
		TargetPlayer = OtherActor;
		GetWorld()->GetTimerManager().SetTimer(DetectionTimer, this, &AChasingEnemy::CheckTargetCondition, DetectionInterval, true);
	}
}

void AChasingEnemy::OnRecogRangeExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	if (IsValid(OtherActor) && OtherActor->IsA(ADrone::StaticClass()) && IsValid(OtherComp) && OtherComp->IsA(USphereComponent::StaticClass())) {
		UE_LOG(LogTemp, Warning, TEXT("Exit!"));
		TargetPlayer = nullptr;
		if (GetWorld()->GetTimerManager().IsTimerActive(DetectionTimer)) {
			GetWorld()->GetTimerManager().ClearTimer(DetectionTimer);
		}
	}
}