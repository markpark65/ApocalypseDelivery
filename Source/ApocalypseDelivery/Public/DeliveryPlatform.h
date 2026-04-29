#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DeliveryPlatform.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
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
protected:
	virtual void BeginPlay() override;
    //virtual void Tick(float DeltaTime) override;
    // 영역 감지용 콜리전
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UBoxComponent* DeliveryZone;

    // 플랫폼 외형
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UStaticMeshComponent* PlatformMesh;

    // 상자가 들어왔을 때 호출될 함수
    virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

    // 성공 시 재생할 사운드
    UPROPERTY(EditAnywhere, Category = "Effects")
    class USoundBase* SuccessSound;
    
    // 나이아가라 이펙트
    UPROPERTY(VisibleAnywhere, Category = "Effects")
    UNiagaraComponent* DirectionComp;

    UPROPERTY(VisibleAnywhere, Category = "Effects")
    UNiagaraComponent* SuccessComp;
};
