// Fill out your copyright notice in the Description page of Project Settings.


#include "ChasingEnemy.h"
#include "Components/StaticMeshComponent.h"
//#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"

#include "ItemMessageWidget.h"
#include "Drone.h"

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
	
}

// Called when the game starts or when spawned
void AChasingEnemy::BeginPlay()
{
	Super::BeginPlay();
	Super::BeginPlay();
	BasePosition = GetActorLocation();
	IsChasing = false;
	TargetPlayer = nullptr;

	GetWorld()->GetTimerManager().SetTimer(DetectionTimer, this, &AChasingEnemy::CheckTargetCondition, DetectionInterval, true);
	//SphereComp->OnComponentHit.AddDynamic(this, &AChasingEnemy::OnCollision);
}

// Called every frame
void AChasingEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (IsChasing)
	{
		AActor* MovementTarget = Cast<ADrone>(TargetPlayer);

		if (!IsValid(MovementTarget)) return;

		DrawDebugLine(GetWorld(), GetActorLocation(), MovementTarget->GetActorLocation(), FColor::Red, false, 0.017);
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
	DrawDebugLine(GetWorld(), GetActorLocation(), TargetPlayer->GetActorLocation(), FColor::Red, false, 0.017);
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
	DrawDebugSphere(GetWorld(), BasePosition, SearchRange, 32, FColor::Cyan, false, DetectionInterval);
	for (AActor* Actor : OverlappedActors) {
		ADrone* Drone = Cast<ADrone>(Actor);
		if (!IsValid(Drone)) {
			continue;
		}
		FHitResult Hit;
		FCollisionQueryParams TraceParams;
		TraceParams.AddIgnoredActor(this);

		DrawDebugLine(GetWorld(), GetActorLocation(), Drone->GetActorLocation(), FColor::Red, false, DetectionInterval);
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

/*
void AChasingEnemy::OnCollision(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) {
	UE_LOG(LogTemp, Warning, TEXT("Enemy Collided!"));
	if (OtherActor->IsA(ADrone::StaticClass())) {
		UE_LOG(LogTemp, Warning, TEXT("Applying debuff!"));
		Cast<ADrone>(OtherActor)->SetReverseControl(10.0f);
	}
	Destroy();
}*/

void AChasingEnemy::PlayOverlapEffects() {
		if (OverlapParticle) UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), OverlapParticle, GetActorLocation());
		if (OverlapSound) UGameplayStatics::PlaySoundAtLocation(this, OverlapSound, GetActorLocation());
}

void AChasingEnemy::ShowPickupUI(ADrone* Drone) {
	// 드론이 없거나, 위젯 클래스가 설정되지 않았거나, 메시지가 비어있으면 실행하지 않음
	if (!Drone || !ItemMessageWidgetClass || PickupMessage.IsEmpty()) return;

	APlayerController* PC = Cast<APlayerController>(Drone->GetController());
	if (PC)
	{
		// 위젯 생성
		UUserWidget* RawWidget = CreateWidget<UUserWidget>(PC, ItemMessageWidgetClass);
		UItemMessageWidget* ItemWidget = Cast<UItemMessageWidget>(RawWidget);

		if (ItemWidget)
		{
			// 메시지 설정 및 화면에 추가
			ItemWidget->SetMessage(PickupMessage);
			ItemWidget->AddToViewport();

			UE_LOG(LogTemp, Warning, TEXT("ItemCollided!"));
			// 2초 뒤에 위젯을 제거하는 타이머 설정
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, [ItemWidget]() {
				if (ItemWidget) ItemWidget->RemoveFromParent();
				}, 2.0f, false);
		}
	}
}

void AChasingEnemy::ApplyEffect_Implementation(class ADrone* Drone) {
	//Drone->SetReverseControl(10.0f);
	PlayOverlapEffects();
	ShowPickupUI(Drone);
	Destroy();
}