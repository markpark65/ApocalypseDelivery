#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DeliveryPlatform.generated.h"

UCLASS()
class APOCALYPSEDELIVERY_API ADeliveryPlatform : public AActor
{
	GENERATED_BODY()
	
public:	
	ADeliveryPlatform();

    UFUNCTION(BlueprintCallable, Category = "Delivery")
    void MarkAsUsed();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Delivery")
    bool bIsUsed = false;
    UFUNCTION(BlueprintCallable, Category = "Delivery")
    void SetIsTarget(bool bIsTarget);
protected:
	virtual void BeginPlay() override;
    // 영역 감지용 콜리전
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UBoxComponent* DeliveryZone;

    // 플랫폼 외형
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UStaticMeshComponent* PlatformMesh;
    //UPROPERTY(VisibleAnywhere, Category = "Components")
    //UStaticMeshComponent* TargetIndicatorMesh;

    // 상자가 들어왔을 때 호출될 함수
    virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

    // 성공 시 재생할 사운드
    UPROPERTY(EditAnywhere, Category = "Effects")
    class USoundBase* SuccessSound;
    
    virtual void Tick(float DeltaTime) override;

    // 회전 및 상하 이동 속도 조절용 변수
    UPROPERTY(EditAnywhere, Category = "Effects")
    float RotationSpeed = 100.0f;

    UPROPERTY(EditAnywhere, Category = "Effects")
    float BobbingSpeed = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Effects")
    float BobbingAmount = 50.0f;

    float InitialRelativeZ;
};
