// Fill out your copyright notice in the Description page of Project Settings.


#include "RotatingObstacle.h"

// Sets default values
ARotatingObstacle::ARotatingObstacle()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
}

// Called when the game starts or when spawned
void ARotatingObstacle::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARotatingObstacle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	AddActorLocalRotation(FRotator(0,0,RotationSpeed * DeltaTime));
}

