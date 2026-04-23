#include "Drone.h"
#include "ApocalypseDroneController.h"
#include "ADInteractable.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "ApocalypseHUD.h"
#include "ApocalypseGameMode.h"

#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Camera/CameraComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"


ADrone::ADrone()
	: AttachedPackage(nullptr)
	, CapsuleHalfHeight(0.0f)
	, CurrentBattery(0.0f)
	, OriginalSpeed(0.0f)
	, bIsOnGround(false)
{
	PrimaryActorTick.bCanEverTick = true;

	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	SetRootComponent(CapsuleComp);
	CapsuleComp->SetSimulatePhysics(false);

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetSimulatePhysics(false);

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 400.0f;
	SpringArmComp->bUsePawnControlRotation = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->SetSphereRadius(150.f);

	ShieldMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShieldMesh"));
	ShieldMesh->SetupAttachment(RootComponent);
	ShieldMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ShieldMesh->SetVisibility(false);

	MovementComp = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComp"));
	MovementComp->SetUpdatedComponent(CapsuleComp);

	PhysicsConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("PhysicalConstComp"));
	PhysicsConstraint->SetupAttachment(RootComponent);

	CapsuleComp->OnComponentHit.AddDynamic(this, &ADrone::OnDroneHit);
}

void ADrone::BeginPlay()
{
	Super::BeginPlay();
	
	CapsuleHalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();
	OriginalSpeed = MoveSpeed;

	CurrentBattery = MaxBattery;
	IsMoving = false;
	DesiredDirection = { 0,0,0 }; 
	GM = Cast<AApocalypseGameMode>(GetWorld()->GetAuthGameMode());

	// 쉴드 초기 상태 설정
	if (ShieldMesh)
	{
		ShieldMesh->SetVisibility(false);
	}
}
void ADrone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
	if (GM->IsTimerActive())
	{
		//Calculate velocity and apply to the movement
		CurrentDirection = FMath::VInterpTo(CurrentDirection, DesiredDirection, DeltaTime, MovementLerpRate);
		CurrentBattery -= CurrentDirection.Length() * DeltaTime;
		if (CurrentBattery <= 0)
		{
			CurrentBattery = 0;
			HandleGameOver();
		}
		AddMovementInput(CurrentDirection);
	}
	


	//Rotate pawn
	FRotator TargetRotation(0, 0, 0);
	//Pitch, changed by velocity
	if (IsMoving) {
		double TiltDirection = FVector::DotProduct(CurrentDirection, CameraComp->GetForwardVector());
		double RollDirection = FVector::DotProduct(CurrentDirection, CameraComp->GetRightVector());
		TargetRotation.Pitch = -TiltDirection * GetMovementComponent()->GetMaxSpeed() / VelocityTiltRatio;
		TargetRotation.Roll = RollDirection * GetMovementComponent()->GetMaxSpeed() / VelocityTiltRatio;
	}
	else {
		TargetRotation.Pitch = 0;
	}
	//Yaw, always chase the camera.
	TargetRotation.Yaw = CameraComp->GetComponentRotation().Yaw;
	//Roll, reset to 0 while not rolling.
	UE_LOG(LogTemp, Warning, TEXT("Rolling - %d"), IsRolling);
	if (!IsRolling) {
		//TargetRotation.Roll = RollDirection * GetMovementComponent()->GetMaxSpeed() / VelocityTiltRatio;
	}
	else {
		TargetRotation.Roll = GetActorRotation().Roll;
	}
	SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, RotationLerpRate));
}

void ADrone::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (AApocalypseDroneController* PC = Cast<AApocalypseDroneController>(GetController()))
		{
			EnhancedInput->BindAction(PC->MoveAction, ETriggerEvent::Triggered, this, &ADrone::BeginMove);
			EnhancedInput->BindAction(PC->MoveAction, ETriggerEvent::Completed, this, &ADrone::EndMove);
			EnhancedInput->BindAction(PC->LookAction, ETriggerEvent::Triggered, this, &ADrone::Look);
			//EnhancedInput->BindAction(PC->UpDownAction, ETriggerEvent::Triggered, this, &ADrone::UpDown);
			EnhancedInput->BindAction(PC->RollAction, ETriggerEvent::Triggered, this, &ADrone::BeginRolling);
			EnhancedInput->BindAction(PC->RollAction, ETriggerEvent::Completed, this, &ADrone::EndRolling);
			EnhancedInput->BindAction(PC->PickupAction, ETriggerEvent::Triggered, this, &ADrone::Pickup);
		}
	}
}
void ADrone::BeginMove(const FInputActionValue& Value)
{
	if (!Controller) return;
	IsMoving = true;
	//Set desired movement direction to unit vector of local coordinate
	FVector Input = Value.Get<FVector>();
	FVector Direction(0.0);
	if (!FMath::IsNearlyZero(Input.X)) {
		Direction += CameraComp->GetForwardVector() * Input.X;
	}
	if (!FMath::IsNearlyZero(Input.Y)) {
		Direction += CameraComp->GetRightVector() * Input.Y;
	}
	if (!FMath::IsNearlyZero(Input.Z)) {
		Direction += FVector(0, 0, Input.Z);
	}
	DesiredDirection = Direction.GetSafeNormal();
}
void ADrone::EndMove(const FInputActionValue& Value)
{
	if (!Controller) return;
	IsMoving = false;
	//Reset desired movement velocity when the input disaapears
	DesiredDirection = { 0, 0, 0 };
}
void ADrone::Look(const FInputActionValue& Value)
{
	if (!Controller) return;
	//Rotate view using mouse input
	FVector2D Input = Value.Get<FVector2D>();
	if (!FMath::IsNearlyZero(Input.X)) {
		AddControllerYawInput(Input.X);
	}
	if (!FMath::IsNearlyZero(Input.Y)) {
		//limit from -90 to 90 degrees

		float CurrentPitch = FRotator::NormalizeAxis(GetControlRotation().Pitch);
		//if ((Input.Y < 0 && CurrentPitch + Input.Y > -85) || (Input.Y > 0 && CurrentPitch + Input.Y < 85.0)) {
		AddControllerPitchInput(Input.Y);
		//}
	}
}
void ADrone::BeginRolling(const FInputActionValue& Value)
{
	if (!Controller) return;
	//Add Roll Rotation
	float Input = Value.Get<float>();
	if (!FMath::IsNearlyZero(Input)) {
		AddActorLocalRotation(FRotator(0, 0, Input * RollingSpeed * GetWorld()->GetDeltaSeconds()));
		IsRolling = true;	//Rolling Status
	}
}

void ADrone::EndRolling(const FInputActionValue& Value)
{
	if (!Controller) return;
	//Clear Rolling Status
	IsRolling = false;
}
void ADrone::Pickup(const FInputActionValue& Value)
{
	if (AttachedPackage)
	{
		// 부착 해제
		AActor* PackageToDrop = AttachedPackage;
		AttachedPackage = nullptr;
		PackageToDrop->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		if (UPrimitiveComponent* PackageMesh = Cast<UPrimitiveComponent>(PackageToDrop->GetRootComponent()))
		{
			// 물리 및 충돌 설정 복구
			PackageMesh->SetSimulatePhysics(true);
			PackageMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			PackageMesh->SetCollisionProfileName(TEXT("PhysicsActor"));

			//다시 잡는 현상 방지
			FVector DropLocation = GetActorLocation() + (GetActorUpVector() * -150.0f);
			PackageToDrop->SetActorLocation(DropLocation);
			// 날아가는 현상 방지
			PackageMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
			PackageMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

			if (IsValid(GM))
			{
				if (GM->CurrentHUD) GM->CurrentHUD->SetInteractionPrompt(true, TEXT("Press F to Pickup"));
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("Package Released safely!"));
	}
	else
	{
		TArray<AActor*> OverlappingActors;
		InteractionSphere->GetOverlappingActors(OverlappingActors);

		for (AActor* Actor : OverlappingActors)
		{
			// 인터페이스(IADInteractable)나 태그를 확인
			if (Actor->ActorHasTag("Package"))
			{
				AttachedPackage = Actor;
				if (UPrimitiveComponent* PackageMesh = Cast<UPrimitiveComponent>(AttachedPackage->GetRootComponent()))
				{
					// 부착 시에는 물리를 끄고 충돌을 비활성화
					PackageMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
					PackageMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

					PackageMesh->SetSimulatePhysics(false);

					PackageMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					PackageMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
				}

				// 드론 하단 소켓 부착
				AttachedPackage->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				AttachedPackage->SetActorRelativeLocation(FVector(0, 0, -100.f));

				if (IsValid(GM))
				{
					if (GM->CurrentHUD) GM->CurrentHUD->SetInteractionPrompt(true, TEXT("Press F to Drop"));
				}
				break;
			}
		}
	}
}
void ADrone::SetTemporarySpeed(float Multiplier, float Duration)
{
	MoveSpeed = OriginalSpeed * Multiplier;

	GetWorldTimerManager().SetTimer(SpeedTimerHandle, this, &ADrone::ResetSpeed, Duration, false);
	UE_LOG(LogTemp, Warning, TEXT("Speed Changed! Multiplier: %f"), Multiplier);
}

void ADrone::ResetSpeed()
{
	MoveSpeed = OriginalSpeed;
	UE_LOG(LogTemp, Warning, TEXT("Speed Restored."));
}

// 배리어 설정
void ADrone::SetShield(bool bEnable)
{
	bHasShield = bEnable;
	if (ShieldMesh)
	{
		ShieldMesh->SetVisibility(bEnable);
	}
	UE_LOG(LogTemp, Warning, TEXT("Shield Status: %s"), bEnable ? TEXT("ON") : TEXT("OFF"));
}

// 조장 방해 설정
void ADrone::SetReverseControl(float Duration)
{
	bIsReverseControl = true;
	GetWorldTimerManager().SetTimer(ReverseTimerHandle, this, &ADrone::ResetReverseControl, Duration, false);
	UE_LOG(LogTemp, Warning, TEXT("Reverse Control Active!"));
}
void ADrone::ResetReverseControl() { bIsReverseControl = false; }

//카메라 고정 설정
void ADrone::SetLookFreeze(float Duration)
{
	bIsLookFrozen = true;
	GetWorldTimerManager().SetTimer(LookFreezeTimerHandle, this, &ADrone::ResetLookFreeze, Duration, false);
	UE_LOG(LogTemp, Warning, TEXT("Camera Frozen!"));
}

void ADrone::ResetLookFreeze() { bIsLookFrozen = false; }

//디버프 제거
void ADrone::ClearAllDebuffs()
{
	bIsReverseControl = false;
	bIsLookFrozen = false;

	GetWorldTimerManager().ClearTimer(ReverseTimerHandle);
	GetWorldTimerManager().ClearTimer(LookFreezeTimerHandle);

	UE_LOG(LogTemp, Warning, TEXT("All Debuffs Cleared!"));
}

void ADrone::AddBattery(float Amount)
{
	CurrentBattery = FMath::Clamp(CurrentBattery + Amount, 0.0f, MaxBattery);
}

//운석 충돌 로직
void ADrone::OnDroneHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor && OtherActor->ActorHasTag("Meteor"))
	{
		if (bHasShield)
		{
			// 쉴드가 있으면 쉴드만 파괴하고 운석 제거
			SetShield(false);
			OtherActor->Destroy();
			UE_LOG(LogTemp, Warning, TEXT("Shield Blocked Meteor!"));
		}
		else
		{
			// 쉴드 없으면 게임 오버
			HandleGameOver();
		}
	}
}
void ADrone::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (!OtherActor) return;

	if (OtherActor->ActorHasTag("Package"))
	{
		if (IsValid(GM))
		{
			if (GM->CurrentHUD)
			{
				FString Message = AttachedPackage ? TEXT("Press F to Drop") : TEXT("Press F to Pickup");
				GM->CurrentHUD->SetInteractionPrompt(true, Message);
			}
		}
	}

	IADInteractable* Interactable = Cast<IADInteractable>(OtherActor);
	if (Interactable)
	{
		UE_LOG(LogTemp, Warning, TEXT("Item Overlapped: %s"), *OtherActor->GetName());
		Interactable->ApplyEffect(this);
	}
}
void ADrone::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);

	if (OtherActor && OtherActor->ActorHasTag("Package"))
	{
		if (IsValid(GM))
		{
			if (GM->CurrentHUD)
			{
				GM->CurrentHUD->SetInteractionPrompt(false, TEXT(""));
			}
		}
	}
}
void ADrone::HandleGameOver()
{
	UE_LOG(LogTemp, Error, TEXT("GAME OVER! Hit by Meteor."));

	// 드론 조작 금지
	DisableInput(Cast<APlayerController>(GetController()));
	//CurrentVelocity = FVector::ZeroVector;
	SetActorHiddenInGame(true);

	if (AttachedPackage)
	{
		AttachedPackage->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		if (auto* Mesh = Cast<UPrimitiveComponent>(AttachedPackage->GetRootComponent()))
			Mesh->SetSimulatePhysics(true);
	}
	GetWorldTimerManager().SetTimer(GameOverTimerHandle, this, &ADrone::DelayedGameOver, 2.0f, false);
}
void ADrone::DelayedGameOver()
{
	if (IsValid(GM))
	{
		GM->EndGame(false);
	}
}