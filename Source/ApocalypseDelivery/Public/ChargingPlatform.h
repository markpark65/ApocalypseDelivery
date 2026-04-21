#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChargingPlatform.generated.h"

UCLASS()
class APOCALYPSEDELIVERY_API AChargingPlatform : public AActor
{
	GENERATED_BODY()
	
public:
	AChargingPlatform();

protected:
	virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UBoxComponent* ChargingZone;

    // 플랫폼 외형
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UStaticMeshComponent* PlatformMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    float RecoverBattery = 10.0f;
    // 재생할 사운드
    UPROPERTY(EditAnywhere, Category = "Effects")
    class USoundBase* RecoverSound;
};
