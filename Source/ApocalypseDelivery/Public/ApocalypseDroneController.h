#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ApocalypseDroneController.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS()
class APOCALYPSEDELIVERY_API AApocalypseDroneController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AApocalypseDroneController();

	// IMC
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputMappingContext* InputMappingContext;

	// 기존 액션
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* LookAction;

	// 추가 액션 (비행용)
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	//UInputAction* UpDownAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* RollAction;

	// 택배 들기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* PickupAction;

	// 택배 들기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* IA_Interact;


	//카메라 효과
	UFUNCTION(Blueprintcallable)
	void SetCameraFade(float duration);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	TSubclassOf<UCameraShakeBase> ShakeBase;

	void ShakeCamera(float ImpactScale);
protected:
	virtual void BeginPlay() override;

	FTimerHandle FadeTimerHandle;
	void ResetCameraFade();
};
