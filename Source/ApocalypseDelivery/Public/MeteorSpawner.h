#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MeteorSpawner.generated.h"

UCLASS()
class APOCALYPSEDELIVERY_API AMeteorSpawner : public AActor
{
	GENERATED_BODY()
	
public:
    AMeteorSpawner();

    UFUNCTION(BlueprintCallable, Category = "Spawning")
    void StartSpawning();

    UFUNCTION(BlueprintCallable, Category = "Spawning")
    void StopSpawning();

    // 난이도 조절
    void SetSpawnInterval(float NewInterval);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, Category = "Spawning")
    TSubclassOf<AActor> MeteorClass;

    UPROPERTY(EditAnywhere, Category = "Spawning")
    float SpawnInterval = 2.0f;

    void SpawnMeteor();

private:
    FTimerHandle SpawnTimerHandle;

};
