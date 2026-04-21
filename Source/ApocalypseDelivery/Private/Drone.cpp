#include "Drone.h"
#include "ApocalypseDroneController.h"
#include "ADInteractable.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/SphereComponent.h"
#include "ApocalypseHUD.h"
#include "ApocalypseGameMode.h"

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
	SpringArmComp->bUsePawnControlRotation = false;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->SetSphereRadius(150.f);

	ShieldMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShieldMesh"));
	ShieldMesh->SetupAttachment(RootComponent);
	ShieldMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ShieldMesh->SetVisibility(false);

	CapsuleComp->OnComponentHit.AddDynamic(this, &ADrone::OnDroneHit);
}

void ADrone::BeginPlay()
{
	Super::BeginPlay();
	
	CapsuleHalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();
	OriginalSpeed = MoveSpeed;

	CurrentBattery = MaxBattery;

	// 쉴드 초기 상태 설정
	if (ShieldMesh)
	{
		ShieldMesh->SetVisibility(false);
	}
}
void ADrone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//  및 속도 계산
	FVector TargetAcceleration = FVector(MovementInput.X, MovementInput.Y, MovementInput.Z);
	CurrentVelocity += TargetAcceleration * Acceleration * DeltaTime;

	// 마찰력 적용
	CurrentVelocity -= CurrentVelocity * Friction * DeltaTime;

	CurrentVelocity = CurrentVelocity.GetClampedToSize(0.f, MoveSpeed);

	//이동 벡터 계산
	FVector DeltaLocation = (GetActorForwardVector() * CurrentVelocity.X +
		GetActorRightVector() * CurrentVelocity.Y +
		GetActorUpVector() * CurrentVelocity.Z) * DeltaTime;

	//회전 처리
	if (!RotationInput.IsZero())
	{
		FRotator NewRotation = RotationInput * RotationSpeed * DeltaTime;
		AddActorLocalRotation(NewRotation);
	}

	// 충돌 판정
	FHitResult HitResult;

	// 벽을 뚫지 않도록 함
	AddActorWorldOffset(DeltaLocation, true, &HitResult);

	//충돌 시 속도 감쇄
	if (HitResult.IsValidBlockingHit())
	{
		// 충돌한 면Normal을 기준 속도를 투영하여 뚫림 방지
		CurrentVelocity = FVector::VectorPlaneProject(CurrentVelocity, HitResult.Normal);

		// 바닥 판정 Z축 법선이 위를 향할 때
		bIsOnGround = HitResult.ImpactNormal.Z > 0.5f;
	}
	else
	{
		bIsOnGround = false;
	}

	//입력값 초기화
	MovementInput = FVector::ZeroVector;
	RotationInput = FRotator::ZeroRotator;
	float SpeedRatio = CurrentVelocity.Size() / MoveSpeed;
	float DrainRate = 1.0f + (SpeedRatio * 2.0f);
	if (AApocalypseGameMode* GM = Cast<AApocalypseGameMode>(GetWorld()->GetAuthGameMode()))
	{
		// 퀘스트가 시작되어 타이머가 돌고 있을 때만 배터리 감소
		if (GM->IsTimerActive())
		{
			CurrentBattery -= DrainRate * DeltaTime;

			if (CurrentBattery <= 0)
			{
				CurrentBattery = 0;
				HandleGameOver();
			}
		}
	}
}

void ADrone::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (AApocalypseDroneController* PC = Cast<AApocalypseDroneController>(GetController()))
		{
			EnhancedInput->BindAction(PC->MoveAction, ETriggerEvent::Triggered, this, &ADrone::Move);
			EnhancedInput->BindAction(PC->LookAction, ETriggerEvent::Triggered, this, &ADrone::Look);
			EnhancedInput->BindAction(PC->UpDownAction, ETriggerEvent::Triggered, this, &ADrone::UpDown);
			EnhancedInput->BindAction(PC->RollAction, ETriggerEvent::Triggered, this, &ADrone::Roll);
			EnhancedInput->BindAction(PC->PickupAction, ETriggerEvent::Triggered, this, &ADrone::Pickup);
		}
	}
}
void ADrone::Move(const FInputActionValue& Value)
{
	if (auto* GM = Cast<AApocalypseGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (!GM->IsTimerActive()) return;
	}
	FVector2D Input = Value.Get<FVector2D>();
	float Modifier = bIsReverseControl ? -1.0f : 1.0f;
	MovementInput.X = Input.X * Modifier;
	MovementInput.Y = Input.Y * Modifier;
}
void ADrone::Look(const FInputActionValue& Value)
{

	if (bIsLookFrozen) return;
	FVector2D Input = Value.Get<FVector2D>();
	RotationInput.Yaw = Input.X;
	RotationInput.Pitch = -Input.Y;
}
void ADrone::UpDown(const FInputActionValue& Value)
{
	if (auto* GM = Cast<AApocalypseGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (!GM->IsTimerActive()) return;
	}
	float Modifier = bIsReverseControl ? -1.0f : 1.0f;
	MovementInput.Z = Value.Get<float>() * Modifier;
}
void ADrone::Roll(const FInputActionValue& Value)
{
	RotationInput.Roll = Value.Get<float>();
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

			if (auto* GM = Cast<AApocalypseGameMode>(GetWorld()->GetAuthGameMode()))
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

				if (auto* GM = Cast<AApocalypseGameMode>(GetWorld()->GetAuthGameMode()))
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
		if (AApocalypseGameMode* GM = Cast<AApocalypseGameMode>(GetWorld()->GetAuthGameMode()))
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
		if (auto* GM = Cast<AApocalypseGameMode>(GetWorld()->GetAuthGameMode()))
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
	CurrentVelocity = FVector::ZeroVector;
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
	if (AApocalypseGameMode* GM = Cast<AApocalypseGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->EndGame(false);
	}
}