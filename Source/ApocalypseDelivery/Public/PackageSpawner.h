#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PackageSpawner.generated.h"

UCLASS()
class APOCALYPSEDELIVERY_API APackageSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
    UPROPERTY(EditAnywhere, Category = "Spawning")
    TSubclassOf<AActor> PackageClass;

    // 상자를 새로 스폰
	UFUNCTION(BlueprintCallable)
    void SpawnPackage();

};
