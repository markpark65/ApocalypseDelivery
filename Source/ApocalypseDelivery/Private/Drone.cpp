#include "Drone.h"
#include "ApocalypseDroneController.h"
#include "ADInteractable.h"
#include "ApocalypseHUD.h"
#include "ApocalypseGameMode.h"
#include "ApocalypseGameStateBase.h"
#include "PackageSpawner.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "ApocalypseHUD.h"
#include "ApocalypseGameMode.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"

#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Camera/CameraComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Kismet/GameplayStatics.h"


ADrone::ADrone()
	: AttachedPackage(nullptr)
	//, CapsuleHalfHeight(0.0f)
	//, CurrentBattery(0.0f)
	//, OriginalSpeed(0.0f)
	//, bIsOnGround(false)
{
	PrimaryActorTick.bCanEverTick = true;

	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("CapsuleComp"));
	SetRootComponent(BoxComp);
	BoxComp->SetSimulatePhysics(false);

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
	MovementComp->SetUpdatedComponent(BoxComp);

	PhysicsConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("PhysicalConstComp"));
	PhysicsConstraint->SetupAttachment(RootComponent);

	// ── 미니맵 SceneCapture 설정 ──
	MinimapCaptureComp = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("MinimapCaptureComp"));
	MinimapCaptureComp->SetupAttachment(RootComponent);

	// 드론 바로 위 2000cm에서 수직 하향 촬영
	MinimapCaptureComp->SetRelativeLocation(FVector(0.f, 0.f, 2000.f));
	MinimapCaptureComp->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));

	// ── 알파 채널을 무시하고 불투명하게 렌더링 ──
	MinimapCaptureComp->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	MinimapCaptureComp->TextureTarget = MinimapRenderTarget;

	// 직교 투영으로 왜곡 없는 탑다운 뷰
	MinimapCaptureComp->ProjectionType = ECameraProjectionMode::Orthographic;
	MinimapCaptureComp->OrthoWidth = MinimapOrthoWidth;

	// 매 프레임 갱신 (성능이 걱정되면 false로 바꾸고 필요 시 CaptureScene() 수동 호출)
	MinimapCaptureComp->bCaptureEveryFrame = true;
	MinimapCaptureComp->bCaptureOnMovement = false;
}

void ADrone::BeginPlay()
{
	Super::BeginPlay();
	
	//CapsuleHalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();
	//OriginalSpeed = MoveSpeed;

	//CurrentBattery = MaxBattery;
	IsMoving = false;
	ControlMultiplier = 1.0f;
	OriginalArmLength = SpringArmComp->TargetArmLength;
	DesiredDirection = FVector::ZeroVector;
	HasTeleport = false;
	TeleportCoordinate = FVector::ZeroVector;
	OriginalSpeed = MovementComp->MaxSpeed;
	OriginalMovementLerpRate = MovementLerpRate;
	OriginalRotationLerpRate = RotationLerpRate;
	GM = Cast<AApocalypseGameMode>(GetWorld()->GetAuthGameMode());

	// 쉴드 초기 상태 설정
	if (ShieldMesh)
	{
		ShieldMesh->SetVisibility(false);
	}
	BoxComp->OnComponentHit.AddDynamic(this, &ADrone::OnDroneHit);

	// ── 미니맵 RenderTarget 런타임 생성 ──
	MinimapRenderTarget = NewObject<UTextureRenderTarget2D>(this, TEXT("MinimapRenderTarget"));
	MinimapRenderTarget->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
	MinimapRenderTarget->InitAutoFormat(MinimapTextureSize, MinimapTextureSize);
	MinimapRenderTarget->UpdateResourceImmediate(true);

	// SceneCapture에 타겟 연결
	if (MinimapCaptureComp)
	{
		// 1. 드론의 회전을 따라가지 않도록 절대 회전 설정
		MinimapCaptureComp->SetUsingAbsoluteRotation(true);

		// 2. 카메라가 항상 땅바닥을 수직으로 내려다보도록 회전값 고정
		// Pitch -90 (바닥), Yaw 0 (북쪽), Roll 0
		MinimapCaptureComp->SetWorldRotation(FRotator(-90.0f, 0.0f, 0.0f));

		MinimapCaptureComp->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
		if (MinimapRenderTarget)
		{
			// 렌더링 결과에서 알파 채널이 배경을 뚫지 않도록 합니다.
			MinimapCaptureComp->TextureTarget = MinimapRenderTarget;
		}
	}

	// GameMode → HUD → MinimapWidget에 RenderTarget 전달
	if (AApocalypseGameMode* MGM = Cast<AApocalypseGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (MGM->CurrentHUD)
		{
			MGM->CurrentHUD->InitializeMinimap(MinimapRenderTarget);
		}
	}

	// ── 정적 텍스처 사용 시 초기화 흐름 ──
	// 위 RenderTarget 흐름 대신 아래 코드를 사용
	/*
	if (StaticMinimapTexture)
	{
		if (AApocalypseGameMode* GM = Cast<AApocalypseGameMode>(GetWorld()->GetAuthGameMode()))
		{
			if (GM->CurrentHUD)
			{
				GM->CurrentHUD->InitializeMinimapWithTexture(StaticMinimapTexture);
			}
		}
	}
	*/
	// ── 
	
	//시작과 동시에 상자 보유.
	AActor* FoundSpawner = UGameplayStatics::GetActorOfClass(GetWorld(), APackageSpawner::StaticClass());
	if (APackageSpawner* PSpawner = Cast<APackageSpawner>(FoundSpawner))
	{
		PSpawner->SpawnPackage();
	}
	TArray<AActor*> Packages;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Package"), Packages);
	if (!Packages.IsEmpty()) {
		UE_LOG(LogTemp, Warning, TEXT("Attaching Package box"));
		Packages[0]->SetActorLocation(GetActorLocation() - GetActorUpVector() * HoldingDistance);
		Pickup(FInputActionValue());
	}
}
void ADrone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
	if (GM->IsTimerActive())
	{
		//Calculate velocity and apply to the movement
		CurrentDirection = FMath::VInterpTo(CurrentDirection, DesiredDirection, DeltaTime, MovementLerpRate);
		/*CurrentBattery -= CurrentDirection.Length() * DeltaTime;
		if (CurrentBattery <= 0)
		{
			CurrentBattery = 0;
			HandleGameOver();
		}*/
		AddMovementInput(CurrentDirection);
	}
	
	// ── 미니맵 관련 코드 ── 
	if (MinimapCaptureComp)
	{
		// 카메라 위치를 드론 정수리 위로 고정 (Z 높이는 캡처 범위에 영향 없음 - Ortho 모드 기준)
		FVector NewLoc = GetActorLocation();
		NewLoc.Z += 5000.0f;
		MinimapCaptureComp->SetWorldLocation(NewLoc);

		// 카메라 회전은 항상 바닥을 향하도록 고정 (드론 회전 무시)
		MinimapCaptureComp->SetWorldRotation(FRotator(-90.0f, 0.0f, 0.0f));
	}
	// ── 

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
			//EnhancedInput->BindAction(PC->PickupAction, ETriggerEvent::Started, this, &ADrone::Pickup);
			EnhancedInput->BindAction(PC->IA_Interact, ETriggerEvent::Started, this, &ADrone::UseItem);
		}
	}
}
void ADrone::BeginMove(const FInputActionValue& Value)
{
	if (!Controller) return;
	IsMoving = true;
	//Set desired movement direction to unit vector of local coordinate
	FVector Input = Value.Get<FVector>() * ControlMultiplier;
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
	FVector2D Input = Value.Get<FVector2D>() * ControlMultiplier;
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
	if (IsValid(AttachedPackage)) {
		return;
	}
	TArray<AActor*> OverlappingActors;
	InteractionSphere->GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		// 인터페이스(IADInteractable)나 태그를 확인
		if (Actor->ActorHasTag("Package"))
		{
			AttachedPackage = Actor;
			break;
		}
	}
	if (!IsValid(AttachedPackage)) {
		return;
	}

	//Set PhysicalConstraint
	UPrimitiveComponent* Target = Cast<UPrimitiveComponent>(AttachedPackage->GetRootComponent());
	AttachedPackage->SetActorLocation(GetActorLocation() - GetActorUpVector() * HoldingDistance);
	PhysicsConstraint->SetConstrainedComponents(Cast<UPrimitiveComponent>(RootComponent), NAME_None, Target, NAME_None);

	PhysicsConstraint->SetLinearXLimit(LCM_Locked, 0.f);
	PhysicsConstraint->SetLinearYLimit(LCM_Locked, 0.f);
	PhysicsConstraint->SetLinearZLimit(LCM_Locked, 0.f);
	PhysicsConstraint->SetAngularSwing1Limit(ACM_Locked, 0.f);
	PhysicsConstraint->SetAngularSwing2Limit(ACM_Locked, 0.f);
	PhysicsConstraint->SetAngularTwistLimit(ACM_Locked, 0.f);
	UE_LOG(LogTemp, Warning, TEXT("Physical Constraints set"));
}
void ADrone::SetTemporarySpeed(float Multiplier, float Duration)
{
	//위젯에 표시할 최댓값 저장
	SpeedEffectMaxDuration = Duration;
	if (Multiplier > 1.0f)
	{
		SpeedEffectName = TEXT("Boost");
	}
	if (Multiplier < 1.0f)
	{
		SpeedEffectName = TEXT("Slow");
	}

	//MoveSpeed = OriginalSpeed * Multiplier;
	if (GetWorld()->GetTimerManager().IsTimerActive(SpeedTimerHandle)) {
		GetWorld()->GetTimerManager().ClearTimer(SpeedTimerHandle);
	}
	MovementComp->MaxSpeed = OriginalSpeed * Multiplier;
	GetWorldTimerManager().SetTimer(SpeedTimerHandle, this, &ADrone::ResetSpeed, Duration, false);
	UE_LOG(LogTemp, Warning, TEXT("Speed Changed! Multiplier: %f"), Multiplier);
}

void ADrone::ResetSpeed()
{
	//MoveSpeed = OriginalSpeed;
	MovementComp->MaxSpeed = OriginalSpeed;
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

void ADrone::SetControlMultiplier(float Muliplier, float Duration) {
	//위젯에 표시할 값 저장
	ControlEffectMaxDuration = Duration;

	ControlMultiplier = Muliplier;
	GetWorldTimerManager().SetTimer(ControlTimerHandle, this, &ADrone::ResetControlMultiplier, Duration, false);
	UE_LOG(LogTemp, Warning, TEXT("Control Changed! - %f"), Muliplier);
}
void ADrone::ResetControlMultiplier() { ControlMultiplier = 1.0f; }
/*
// 조장 방해 설정
void ADrone::SetReverseControl(float Duration)
{
	bIsReverseControl = true;
	GetWorldTimerManager().SetTimer(ReverseTimerHandle, this, &ADrone::ResetReverseControl, Duration, false);
	UE_LOG(LogTemp, Warning, TEXT("Reverse Control Active!"));
}
void ADrone::ResetReverseControl() { bIsReverseControl = false; }*/

//카메라 고정 설정
void ADrone::SetLookFreeze(float Duration)
{
	//위젯에 표시할 최댓값 저장
	LookFreezeMaxDuration = Duration;
	bIsLookFrozen = true;
	GetWorldTimerManager().SetTimer(LookFreezeTimerHandle, this, &ADrone::ResetLookFreeze, Duration, false);
	UE_LOG(LogTemp, Warning, TEXT("Camera Frozen!"));
}

void ADrone::ResetLookFreeze() { bIsLookFrozen = false; }

void ADrone::SetGravitated(float Duration)
{
	//위젯에 표시할 최댓값 저장
	GravityMaxDuration = Duration;
	BoxComp->SetSimulatePhysics(true);
	GetWorldTimerManager().SetTimer(GravityTimerHandle, this, &ADrone::ResetGravited, Duration, false);
	UE_LOG(LogTemp, Warning, TEXT("Gravity on!"));
}

void ADrone::ResetGravited()
{
	BoxComp->SetSimulatePhysics(false);
	UE_LOG(LogTemp, Warning, TEXT("Gravity off!"));
}

void ADrone::SetTemporalScale(float ScaleValue, float CameraDistanceRatio, float Duration) {
	//위젯에 표시할 최댓값 저장
	ScaleMaxDuration = Duration;

	SpringArmComp->TargetArmLength = OriginalArmLength * CameraDistanceRatio;
	SetActorScale3D(FVector(ScaleValue));
	GetWorldTimerManager().SetTimer(ScaleTimerHandle, this, &ADrone::ResetTemporalScale, Duration, false);
	UE_LOG(LogTemp, Warning, TEXT("Scale Changed! - %f"), ScaleValue);
}

void ADrone::ResetTemporalScale() {
	UE_LOG(LogTemp, Warning, TEXT("Scale restored"));
	SetActorScale3D(FVector::OneVector);
	SpringArmComp->TargetArmLength = OriginalArmLength;
}

void ADrone::SetDelayedInput(float MovementDelayRatio, float RotationDelayRatio, float Duration)
{
	//위젯에 표시할 최댓값 저장
	DelayMaxDuration = Duration;

	UE_LOG(LogTemp, Warning, TEXT("LerpRate Delayed: Movement-%f, Rotation %f"), MovementDelayRatio, RotationDelayRatio);
	MovementLerpRate *= MovementDelayRatio;
	RotationLerpRate *= RotationDelayRatio;
	GetWorldTimerManager().SetTimer(ScaleTimerHandle, this, &ADrone::ResetDelayedInput, Duration, false);
}

void ADrone::ResetDelayedInput()
{
	UE_LOG(LogTemp, Warning, TEXT("LerpRate restored - MovementLerpRate: %f, RotationLerpRate: %f"), OriginalMovementLerpRate, OriginalRotationLerpRate);
	MovementLerpRate = OriginalMovementLerpRate;
	RotationLerpRate = OriginalRotationLerpRate;
}

//디버프 제거
void ADrone::ClearAllDebuffs()
{
	//bIsReverseControl = false;
	ControlMultiplier = 1.0f;
	bIsLookFrozen = false;

	GetWorldTimerManager().ClearTimer(ControlTimerHandle);
	GetWorldTimerManager().ClearTimer(LookFreezeTimerHandle);

	UE_LOG(LogTemp, Warning, TEXT("All Debuffs Cleared!"));
}

void ADrone::AddTeleport()
{
	HasTeleport = true;
}

void ADrone::ApplyImpulseVelocity(FVector Impulse)
{
	MovementComp->Velocity += Impulse;
	CurrentDirection = FVector::Zero();
}

/*
void ADrone::AddBattery(float Amount)
{
	CurrentBattery = FMath::Clamp(CurrentBattery + Amount, 0.0f, MaxBattery);
}*/

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

	AApocalypseGameStateBase* GS = GetWorld()->GetGameState<AApocalypseGameStateBase>();
	if (IsValid(GS)) {
		GS->SetNotPlaying();
	}
}
void ADrone::DelayedGameOver()
{
	if (IsValid(GM))
	{
		GM->EndGame(false);
	}
}

void ADrone::UseItem()
{
	if (HasTeleport) {
		if (TeleportCoordinate == FVector::ZeroVector) {
			TeleportCoordinate = GetActorLocation();
			UE_LOG(LogTemp, Warning, TEXT("TeleportCoordinate has set - %f %f %f"), TeleportCoordinate.X, TeleportCoordinate.Y, TeleportCoordinate.Z);
		}
		else {
			SetActorLocation(TeleportCoordinate);
			MovementComp->StopMovementImmediately();
			CurrentDirection = FVector::ZeroVector;
			HasTeleport = false;
			TeleportCoordinate = FVector::ZeroVector;
			UE_LOG(LogTemp, Warning, TEXT("Teleport Completed - %f %f %f"), TeleportCoordinate.X, TeleportCoordinate.Y, TeleportCoordinate.Z);
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Cannot Use Teleport"));
	}
}

//상태변화 Progress Bar 구현 로직
TArray<FEffectUIStatus> ADrone::GetActiveEffectsStatus() const
{
	TArray<FEffectUIStatus> ActiveEffects;
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();

	// 1. 속도 효과 체크
	if (TimerManager.IsTimerActive(SpeedTimerHandle))
	{
		FEffectUIStatus Status;
		Status.EffectName = SpeedEffectName;
		Status.TimeRemaining = TimerManager.GetTimerRemaining(SpeedTimerHandle);
		Status.ProgressRatio = Status.TimeRemaining / SpeedEffectMaxDuration;
		ActiveEffects.Add(Status);
	}

	// 2. 조작 방해 효과 체크
	if (TimerManager.IsTimerActive(ControlTimerHandle))
	{
		FEffectUIStatus Status;
		Status.EffectName = TEXT("Control Reverse");
		Status.TimeRemaining = TimerManager.GetTimerRemaining(ControlTimerHandle);
		Status.ProgressRatio = Status.TimeRemaining / ControlEffectMaxDuration;
		ActiveEffects.Add(Status);
	}

	// 3. 시야 고정 효과 체크
	if (TimerManager.IsTimerActive(LookFreezeTimerHandle))
	{
		FEffectUIStatus Status;
		Status.EffectName = TEXT("Look Freeze");
		Status.TimeRemaining = TimerManager.GetTimerRemaining(LookFreezeTimerHandle);
		Status.ProgressRatio = Status.TimeRemaining / LookFreezeMaxDuration;
		ActiveEffects.Add(Status);
	}

	// 4. 중력 효과 체크
	if (TimerManager.IsTimerActive(GravityTimerHandle))
	{
		FEffectUIStatus Status;
		Status.EffectName = TEXT("Gravitied");
		Status.TimeRemaining = TimerManager.GetTimerRemaining(GravityTimerHandle);
		Status.ProgressRatio = Status.TimeRemaining / GravityMaxDuration;
		ActiveEffects.Add(Status);
	}

	// 5. 스케일 조정 효과 체크
	if (TimerManager.IsTimerActive(ScaleTimerHandle))
	{
		FEffectUIStatus Status;
		Status.EffectName = TEXT("Shirinked");
		Status.TimeRemaining = TimerManager.GetTimerRemaining(ScaleTimerHandle);
		Status.ProgressRatio = Status.TimeRemaining / ScaleMaxDuration;
		ActiveEffects.Add(Status);
	}

	// 6. 입력 지연 효과 체크
	if (TimerManager.IsTimerActive(DelayTimerHandle))
	{
		FEffectUIStatus Status;
		Status.EffectName = TEXT("Input Delayed");
		Status.TimeRemaining = TimerManager.GetTimerRemaining(DelayTimerHandle);
		Status.ProgressRatio = Status.TimeRemaining / DelayMaxDuration;
		ActiveEffects.Add(Status);
	}

	//새로운 상태변화 효과 구현 시 Drone에 로직 추가하고, 이 아래에 같은 구조로 호출할 함수/변수명만 변경하면 됩니다.

	// BP에서 등록한 커스텀 효과들 자동 순회
	for (auto& It : CustomEffectsMap)
	{
		if (TimerManager.IsTimerActive(It.Value.TimerHandle))
		{
			FEffectUIStatus Status;
			Status.EffectName = It.Key;
			Status.TimeRemaining = TimerManager.GetTimerRemaining(It.Value.TimerHandle);
			Status.ProgressRatio = Status.TimeRemaining / It.Value.MaxDuration;
			ActiveEffects.Add(Status);
		}
	}
	return ActiveEffects;
}

// BP에서만 만든 새로운 효과를 UI에 등록할 때 호출할 함수
void ADrone::RegisterCustomEffect(FString EffectName, float Duration)
{
	if (Duration <= 0.0f) return;

	FCustomEffectData& Data = CustomEffectsMap.FindOrAdd(EffectName);
	Data.MaxDuration = Duration;

	// 기존 타이머가 있다면 초기화 후 재설정
	GetWorld()->GetTimerManager().SetTimer(Data.TimerHandle, [this, EffectName]() {
		CustomEffectsMap.Remove(EffectName);
		}, Duration, false);
}