// Fill out your copyright notice in the Description page of Project Settings.


#include "PatrolObstacle.h"
#include "Components/SplineComponent.h"

// Sets default values
APatrolObstacle::APatrolObstacle()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SplineComp = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComp"));
	RootComponent = SplineComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);

	MovementSpeed = 200.0f;
	IsLoop = true;
}

// Called when the game starts or when spawned
void APatrolObstacle::BeginPlay()
{
	Super::BeginPlay();
	CurrentDistance = 0.0f;
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

