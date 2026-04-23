#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "Drone.generated.h"

class UFloatingPawnMovement;
class AApocalypseGameMode;
class UPhysicsConstraintComponent;

UCLASS()
class APOCALYPSEDELIVERY_API ADrone : public APawn
{
	GENERATED_BODY()

public:
	ADrone();

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    bool bHasShield = false;

    //아이템 효과 적용 함수
    void SetTemporarySpeed(float Multiplier, float Duration);
    void SetShield(bool bEnable);

    //조작 전환 아이템
    bool bIsReverseControl = false;
    void SetReverseControl(float Duration);

    //화면 고정 아이템
    bool bIsLookFrozen = false;
    void SetLookFreeze(float Duration);

    // 모든 상태 이상 초기화
    void ClearAllDebuffs();

    // 배터리 회복
    void AddBattery(float Amount);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    float MaxBattery = 100.0f;
    UPROPERTY(BlueprintReadWrite, Category = "Status")
    float CurrentBattery;

    FORCEINLINE FVector GetCurrentVelocity() const { return GetVelocity(); }//return CurrentVelocity; }
    FORCEINLINE AActor* GetAttachedPackage() const {return AttachedPackage; }
    FORCEINLINE void SetAttachedPackage(AActor* NewPackage) { AttachedPackage = NewPackage; }

    void ResetSpeed();
    void HandleGameOver();
protected:
	virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // 컴포넌트
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UCapsuleComponent* CapsuleComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    class USkeletalMeshComponent* MeshComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    class USpringArmComponent* SpringArmComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UCameraComponent* CameraComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UStaticMeshComponent* ShieldMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UFloatingPawnMovement* MovementComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UPhysicsConstraintComponent* PhysicsConstraint;

    // 비행/이동 변수
    UPROPERTY(EditAnywhere, Category = "Movement")
    float MoveSpeed = 600.f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float RotationSpeed = 100.f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float UpDownSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float VelocityTiltRatio;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RotationLerpRate;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MovementLerpRate;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RollingSpeed;

    // 입력 처리 함수
    void BeginMove(const FInputActionValue& Value);
    void EndMove(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    //void UpDown(const FInputActionValue& Value);
    //void Roll(const FInputActionValue& Value);
    void BeginRolling(const FInputActionValue& Value);
    void EndRolling(const FInputActionValue& Value);
    void Pickup(const FInputActionValue& Value);

    // 배달
    UPROPERTY(VisibleAnywhere, Category = "Interaction")
    class USphereComponent* InteractionSphere; // 상자 감지 영역
    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    AActor* AttachedPackage;

    //충돌
    UFUNCTION()
    void OnDroneHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
    virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

    //캐릭터 죽으면 바로 종료 UI 문제
    void DelayedGameOver();
    FTimerHandle GameOverTimerHandle;

private:
    FVector MovementInput = FVector::ZeroVector;
    //FRotator RotationInput = FRotator::ZeroRotator;
    bool IsMoving;
    FVector DesiredDirection;
    FVector CurrentDirection;
    bool IsRolling;
    bool CanMove;

    // 충돌 및 상태 관리를 위한 변수 유지
    float VerticalVelocity = 0.0f;
    float CapsuleHalfHeight;
    bool bIsOnGround = false;

    float OriginalSpeed;

    //조작 방해 아이템
    FTimerHandle ReverseTimerHandle;
    void ResetReverseControl();

    //시야 고정 아이템
    FTimerHandle LookFreezeTimerHandle;
    void ResetLookFreeze();

    // 효과 지속 시간 관리
    FTimerHandle SpeedTimerHandle;
    FTimerHandle DeliveryBlockTimerHandle;
    AApocalypseGameMode* GM;
};