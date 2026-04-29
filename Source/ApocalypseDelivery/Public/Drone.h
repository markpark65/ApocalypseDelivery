#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "Drone.generated.h"

class UFloatingPawnMovement;
class AApocalypseGameMode;
class AApocalypseGameStateBase;
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

    //속도 효과
    UFUNCTION(BlueprintCallable, Exec)
    void SetTemporarySpeed(float Multiplier, float Duration);
    
    //조작 효과
    float ControlMultiplier;
    UFUNCTION(BlueprintCallable)
    void SetControlMultiplier(float Multiplier, float Duration);

    //중력 효과
    bool IsGravitated;
    UFUNCTION(BlueprintCallable)
    void SetGravitated(float Duration);

    //순간 속도 적용
    void ApplyImpulseVelocity(FVector Impulse);

    //스케일 효과
    UFUNCTION(BlueprintCallable)
    void SetTemporalScale(float ScaleValue, float CameraDistanceRatio, float Duration);

    //입력 딜레이 효과
    UFUNCTION(BlueprintCallable)
    void SetDelayedInput(float MovementDelayRatio, float RotationDelayRatio, float Duration);

    //텔레포트 추가
    UFUNCTION(BlueprintCallable)
    void AddTeleport();
    bool ReadyToSetTeleport() { return HasTeleport && TeleportCoordinate == FVector::ZeroVector; };;
    bool ReadyToTeleport() { return HasTeleport && !(TeleportCoordinate == FVector::ZeroVector); };

    FORCEINLINE FVector GetCurrentVelocity() const { return GetVelocity(); }

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

    //택배 상자
    UPROPERTY(EditAnywhere, Category = "Spawning")
    TSubclassOf<AActor> PackageClass;

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

    // 배달
    UPROPERTY(VisibleAnywhere, Category = "Interaction")
    class USphereComponent* InteractionSphere; // 상자 감지 영역
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float HoldingDistance;

    //충돌
    UFUNCTION()
    void OnDroneHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    //virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
    //virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

    //텔레포트
    bool HasTeleport;
    FVector TeleportCoordinate;
    void UseTeleport();

    //드론 SFX
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundBase* DroneSound;
    UPROPERTY()
    UAudioComponent* MovementAudioComp;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundBase* CrushSound;
    UPROPERTY()
    UAudioComponent* CrushAudioComp;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    float SoundMuliplier;
    

    

private:
    FVector MovementInput = FVector::ZeroVector;
    bool IsMoving;
    FVector DesiredDirection;
    FVector CurrentDirection;
    bool IsRolling;

    //조작 효과
    FTimerHandle ControlTimerHandle;
    void ResetControlMultiplier();

    //속도 효과
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
    AApocalypseGameStateBase* GS;

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