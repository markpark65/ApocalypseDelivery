#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "Drone.generated.h"

class UFloatingPawnMovement;
class AApocalypseGameMode;
class UPhysicsConstraintComponent;

// UI로 넘겨줄 상태변화 데이터 구조체 선언
USTRUCT(BlueprintType)
struct FEffectUIStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "UI")
    FString EffectName;       // 텍스트에 띄울 효과 이름

    UPROPERTY(BlueprintReadOnly, Category = "UI")
    float TimeRemaining;      // 남은 시간 (초)

    UPROPERTY(BlueprintReadOnly, Category = "UI")
    float ProgressRatio;      // 프로그레스 바에 넣을 비율 (0.0 ~ 1.0)
};
// BP에서만 구현한 버프/디버프를 위한 부분
USTRUCT(BlueprintType)
struct FCustomEffectData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "CustomEffect")
    float MaxDuration;

    UPROPERTY(BlueprintReadWrite, Category = "CustomEffect")
    FTimerHandle TimerHandle;
};

UCLASS()
class APOCALYPSEDELIVERY_API ADrone : public APawn
{
	GENERATED_BODY()

public:
	ADrone();

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    bool bHasShield = false;

    //아이템 효과 적용 함수
    UFUNCTION(BlueprintCallable)
    void SetTemporarySpeed(float Multiplier, float Duration);
    void SetShield(bool bEnable);
    
    //조작 전환 아이템
    /*bool bIsReverseControl = false;
    UFUNCTION(BlueprintCallable)
    void SetReverseControl(float Duration);*/
    float ControlMultiplier;
    UFUNCTION(BlueprintCallable)
    void SetControlMultiplier(float Multiplier, float Duration);

    //화면 고정 아이템
    bool bIsLookFrozen = false;
    UFUNCTION(BlueprintCallable)
    void SetLookFreeze(float Duration);

    //중력 효과
    bool IsGravitated;
    UFUNCTION(BlueprintCallable)
    void SetGravitated(float Duration);

    //순간 속도 적용
    void ApplyImpulseVelocity(FVector Impulse);

    //스케일 적용
    UFUNCTION(BlueprintCallable)
    void SetTemporalScale(float ScaleValue, float CameraDistanceRatio, float Duration);

    //입력 딜레이 적용
    UFUNCTION(BlueprintCallable)
    void SetDelayedInput(float MovementDelayRatio, float RotationDelayRatio, float Duration);

    // 모든 상태 이상 초기화
    void ClearAllDebuffs();

    //텔레포트 추가
    UFUNCTION(BlueprintCallable)
    void AddTeleport();

    /*
    // 배터리 회복
    void AddBattery(float Amount);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    float MaxBattery = 100.0f;
    UPROPERTY(BlueprintReadWrite, Category = "Status")
    float CurrentBattery;*/

    FORCEINLINE FVector GetCurrentVelocity() const { return GetVelocity(); }//return CurrentVelocity; }
    FORCEINLINE AActor* GetAttachedPackage() const {return AttachedPackage; }
    FORCEINLINE void SetAttachedPackage(AActor* NewPackage) { AttachedPackage = NewPackage; }

    void ResetSpeed();
    void HandleGameOver();

    //UI 위젯에서 상태변화 블루프린트 노드로 호출할 함수
    UFUNCTION(BlueprintPure, Category = "Status|UI")
    TArray<FEffectUIStatus> GetActiveEffectsStatus() const;
    // BP에서만 만든 새로운 효과를 UI에 등록할 때 호출할 함수
    UFUNCTION(BlueprintCallable, Category = "Status|UI")
    void RegisterCustomEffect(FString EffectName, float Duration);
     
    //탑다운 뷰를 그대로 미니맵에 적용
    
    // ── 미니맵 관련 변수──
    //탑다운 SceneCapture – 드론 위 2000cm에서 수직으로 촬영
    UPROPERTY(VisibleAnywhere, Category = "Minimap")
    class USceneCaptureComponent2D* MinimapCaptureComp;

    //SceneCapture가 쓰는 렌더 타겟 (BeginPlay에서 자동 생성)
    UPROPERTY(BlueprintReadOnly, Category = "Minimap")
    class UTextureRenderTarget2D* MinimapRenderTarget;

    //미니맵의 투영 너비.
    UPROPERTY(EditAnywhere, Category = "Minimap")
    float MinimapOrthoWidth = 5000.0f;

    //렌더 타겟 해상도.
    UPROPERTY(EditAnywhere, Category = "Minimap")
    int32 MinimapTextureSize = 256;

    /*
    // ── 별도 약도(정적 텍스처) 사용 시 추가 ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
    class UTexture2D* StaticMinimapTexture;
    */
    
protected:
	virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // 컴포넌트
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UBoxComponent* BoxComp;

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
    //UPROPERTY(EditAnywhere, Category = "Movement")
    //float MoveSpeed = 600.f;

    //UPROPERTY(EditAnywhere, Category = "Movement")
    //float RotationSpeed = 100.f;

    //UPROPERTY(EditAnywhere, Category = "Movement")
    //float UpDownSpeed = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float VelocityTiltRatio;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RotationLerpRate;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MovementLerpRate;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RollingSpeed;

    //카메라 효과
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float FOVChangeRate;

    // 입력 처리 함수
    void BeginMove(const FInputActionValue& Value);
    void EndMove(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void BeginRolling(const FInputActionValue& Value);
    void EndRolling(const FInputActionValue& Value);
    void Pickup(const FInputActionValue& Value);

    // 배달
    UPROPERTY(VisibleAnywhere, Category = "Interaction")
    class USphereComponent* InteractionSphere; // 상자 감지 영역
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
    AActor* AttachedPackage;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float HoldingDistance;

    //충돌
    UFUNCTION()
    void OnDroneHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
    virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

    //캐릭터 죽으면 바로 종료 UI 문제
    void DelayedGameOver();
    FTimerHandle GameOverTimerHandle;

    //텔레포트
    bool HasTeleport;
    FVector TeleportCoordinate;
    void UseItem();

    //드론 이동 SFX
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundBase* DroneSound;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    float SoundMuliplier;
    UPROPERTY()
    UAudioComponent* AudioComp;

    

private:
    FVector MovementInput = FVector::ZeroVector;
    //FRotator RotationInput = FRotator::ZeroRotator;
    bool IsMoving;
    FVector DesiredDirection;
    FVector CurrentDirection;
    bool IsRolling;
    bool CanMove;

    /*
    // 충돌 및 상태 관리를 위한 변수 유지
    float VerticalVelocity = 0.0f;
    float CapsuleHalfHeight;
    bool bIsOnGround = false;

    
    */
    //조작 방해 아이템
    //FTimerHandle ReverseTimerHandle;
    //void ResetReverseControl();
    FTimerHandle ControlTimerHandle;
    void ResetControlMultiplier();

    //시야 고정 아이템
    FTimerHandle LookFreezeTimerHandle;
    void ResetLookFreeze();

    // 효과 지속 시간 관리
    FTimerHandle SpeedTimerHandle;
    float OriginalSpeed;
    FTimerHandle DeliveryBlockTimerHandle;

    //중력 효과
    FTimerHandle GravityTimerHandle;
    void ResetGravited();

    //스케일 효과
    FTimerHandle ScaleTimerHandle;
    float OriginalArmLength;
    void ResetTemporalScale();

    //입력 딜레이 효과
    FTimerHandle DelayTimerHandle;
    float OriginalMovementLerpRate;
    float OriginalRotationLerpRate;
    void ResetDelayedInput();

    AApocalypseGameMode* GM;

    //위젯에 표시할 값 저장
    float SpeedEffectMaxDuration;
    float ControlEffectMaxDuration;
    float LookFreezeMaxDuration;
    float GravityMaxDuration;
    float ScaleMaxDuration;
    float DelayMaxDuration;
    UPROPERTY()
    FString SpeedEffectName;
    // BP에서만 구현한 버프/디버프를 위한 부분
    UPROPERTY()
    TMap<FString, FCustomEffectData> CustomEffectsMap;
};