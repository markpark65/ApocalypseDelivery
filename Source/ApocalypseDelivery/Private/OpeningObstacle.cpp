// Fill out your copyright notice in the Description page of Project Settings.


#include "OpeningObstacle.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Drone.h"

// Sets default values
AOpeningObstacle::AOpeningObstacle()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	RootComponent = BoxComp;

	Door1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door1"));
	Door1->SetupAttachment(RootComponent);

	Door2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door2"));
	Door2->SetupAttachment(RootComponent);

	IsRepeating = true;
	MovedDistance = 0.0f;
	OpeningSpeed = 0.0f;
	OpeningDistance = 0.0f;
}

// Called when the game starts or when spawned
void AOpeningObstacle::BeginPlay()
{
	Super::BeginPlay();
	
	
	if (IsRepeating) {	//자동 반복
		IsOpening = true;
		GetWorld()->GetTimerManager().SetTimer(RepeatTimer, this, &AOpeningObstacle::FlipOpenDirection, OpeningDistance/OpeningSpeed, true);
	}
	else {	//트리거 반복
		IsOpening = false;
		BoxComp->OnComponentBeginOverlap.AddDynamic(this, &AOpeningObstacle::OnBeginOverlap);
		BoxComp->OnComponentEndOverlap.AddDynamic(this, &AOpeningObstacle::OnEndOverlap);
	}
}

void AOpeningObstacle::FlipOpenDirection()
{
	IsOpening = !IsOpening;
}

void AOpeningObstacle::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if(OtherActor->IsA(ADrone::StaticClass()) && OtherComp->IsA(USphereComponent::StaticClass())) FlipOpenDirection();
}

void AOpeningObstacle::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	if (OtherActor->IsA(ADrone::StaticClass()) && OtherComp->IsA(USphereComponent::StaticClass())) FlipOpenDirection();
}

// Called every frame
void AOpeningObstacle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (IsOpening) {
		if (MovedDistance < OpeningDistance) {
			FVector Movement = FVector(0, OpeningSpeed * DeltaTime, 0);
			MovedDistance += OpeningSpeed * DeltaTime;
			Door1->AddLocalOffset(-Movement);
			Door2->AddLocalOffset(Movement);
		}
	}
	else{
		if (MovedDistance >= 0) {
			FVector Movement = FVector(0, OpeningSpeed * DeltaTime, 0);
			MovedDistance -= OpeningSpeed * DeltaTime;
			Door1->AddLocalOffset(Movement);
			Door2->AddLocalOffset(-Movement);
		}

	}
}
