#include "ItemSpawner.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

AItemSpawner::AItemSpawner()
{
    SpawnVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnVolume"));
    RootComponent = SpawnVolume;
}

FVector AItemSpawner::GetRandomLocationInVolume()
{
    if (SpawnVolume)
    {
        return UKismetMathLibrary::RandomPointInBoundingBox(
            SpawnVolume->Bounds.Origin,
            SpawnVolume->Bounds.BoxExtent
        );
    }
    return GetActorLocation(); // 예외 처리
}
void AItemSpawner::SpawnBuffs(int32 Count)
{
    for (int32 i = 0; i < Count; ++i)
    {
        if (BuffClasses.Num() > 0)
        {
            int32 RandomIndex = FMath::RandRange(0, BuffClasses.Num() - 1);
            FVector Location = GetRandomLocationInVolume();
            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            GetWorld()->SpawnActor<AActor>(BuffClasses[RandomIndex], Location, FRotator::ZeroRotator, SpawnParams);
        }
    }
}

void AItemSpawner::SpawnDebuffs(int32 Count)
{
    for (int32 i = 0; i < Count; ++i)
    {
        if (DebuffClasses.Num() > 0)
        {
            int32 RandomIndex = FMath::RandRange(0, DebuffClasses.Num() - 1);
            FVector Location = GetRandomLocationInVolume();
            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            GetWorld()->SpawnActor<AActor>(DebuffClasses[RandomIndex], Location, FRotator::ZeroRotator, SpawnParams);
        }
    }
}

void AItemSpawner::SpawnMines(int32 Count)
{
    for (int32 i = 0; i < Count; ++i)
    {
        if (MineClass)
        {
            FVector Location = GetRandomLocationInVolume();
            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            GetWorld()->SpawnActor<AActor>(MineClass, Location, FRotator::ZeroRotator, SpawnParams);
        }
    }
}