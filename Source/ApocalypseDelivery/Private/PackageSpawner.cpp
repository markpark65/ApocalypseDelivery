#include "PackageSpawner.h"

void APackageSpawner::SpawnPackage()
{
    if (!PackageClass) return;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    GetWorld()->SpawnActor<AActor>(PackageClass, GetActorLocation(), GetActorRotation(), SpawnParams);
}


