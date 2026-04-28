// Fill out your copyright notice in the Description page of Project Settings.


#include "PatrolObstacle.h"
#include "Drone.h"

#include "Components/SplineComponent.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
APatrolObstacle::APatrolObstacle()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SplineComp = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComp"));
	RootComponent = SplineComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);

    WarningBound = CreateDefaultSubobject<USphereComponent>(TEXT("WarningBound"));
    WarningBound->SetupAttachment(MeshComp);

	MovementSpeed = 200.0f;
	IsLoop = true;
}

// Called when the game starts or when spawned
void APatrolObstacle::BeginPlay()
{
	Super::BeginPlay();
	CurrentDistance = 0.0f;

    if (MovementSound)
    {
        MovementAudioComp = UGameplayStatics::SpawnSoundAttached(MovementSound, MeshComp);
    }

    if (WarningSound)
    {
        WarningAudioComp = UGameplayStatics::SpawnSoundAttached(WarningSound, MeshComp);
        if (WarningAudioComp)
        {
            WarningAudioComp->Stop();
        }
    }

    MovementAudioComp = UGameplayStatics::SpawnSoundAttached(MovementSound,MeshComp);
    WarningAudioComp = UGameplayStatics::SpawnSoundAttached(WarningSound, MeshComp);
    WarningAudioComp->Stop();
    WarningBound->OnComponentBeginOverlap.AddDynamic(this, &APatrolObstacle::StartWarning);
    WarningBound->OnComponentEndOverlap.AddDynamic(this, &APatrolObstacle::EndWarning);
}

void APatrolObstacle::StartWarning(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (IsValid(OtherActor) && OtherActor->IsA(ADrone::StaticClass()) && IsValid(WarningAudioComp) && !(WarningAudioComp->IsPlaying())) {
            WarningAudioComp->Play();
    }
}

void APatrolObstacle::EndWarning(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (IsValid(OtherActor) && OtherActor->IsA(ADrone::StaticClass()) && IsValid(WarningAudioComp))
    {
        if (WarningAudioComp->IsPlaying())
        {
            WarningAudioComp->Stop();
        }
    }
    
}

// Called every frame
void APatrolObstacle::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    float SplineLength = SplineComp->GetSplineLength();

    // 거리 누적
    CurrentDistance += MovementSpeed * DeltaTime;

    // 루프 처리
    if (IsLoop) CurrentDistance = FMath::Fmod(CurrentDistance, SplineLength);
    else CurrentDistance = FMath::Clamp(CurrentDistance, 0.f, SplineLength);

    // 스플라인 위치/회전 적용
    FVector NewLocation = SplineComp->GetLocationAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::World);
    FRotator NewRotation = SplineComp->GetRotationAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::World);

    MeshComp->SetWorldLocationAndRotation(NewLocation, NewRotation);
}