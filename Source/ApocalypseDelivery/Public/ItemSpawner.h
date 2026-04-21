#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemSpawner.generated.h"

UCLASS()
class APOCALYPSEDELIVERY_API AItemSpawner : public AActor
{
	GENERATED_BODY()
	
public:
    AItemSpawner();
    void SpawnBuffs(int32 Count);
    void SpawnDebuffs(int32 Count);
    void SpawnMines(int32 Count);

protected:
    UPROPERTY(EditAnywhere, Category = "Spawning")
    TArray<TSubclassOf<AActor>> BuffClasses;

    UPROPERTY(EditAnywhere, Category = "Spawning")
    TArray<TSubclassOf<AActor>> DebuffClasses;

    UPROPERTY(EditAnywhere, Category = "Spawning")
    TSubclassOf<AActor> MineClass;

private:
    UPROPERTY(EditAnywhere)
    class UBoxComponent* SpawnVolume;

    FVector GetRandomLocationInVolume();
};
