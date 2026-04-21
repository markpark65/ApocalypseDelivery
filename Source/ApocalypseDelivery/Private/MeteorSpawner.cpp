#include "MeteorSpawner.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AMeteorSpawner::AMeteorSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

}

void AMeteorSpawner::BeginPlay()
{
    Super::BeginPlay();
    StartSpawning(); 
}

void AMeteorSpawner::StartSpawning()
{
    GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AMeteorSpawner::SpawnMeteor, SpawnInterval, true);
}

void AMeteorSpawner::StopSpawning()
{
    GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
}

void AMeteorSpawner::SetSpawnInterval(float NewInterval)
{
    SpawnInterval = NewInterval;
    if (GetWorldTimerManager().IsTimerActive(SpawnTimerHandle))
    {
        StartSpawning(); // 타이머 갱신
    }
}

void AMeteorSpawner::SpawnMeteor()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn || !MeteorClass) return;

    // 플레이어 위치 파악
    FVector PlayerLoc = PlayerPawn->GetActorLocation();

    // 머리 위 설정
    FVector SpawnLoc = PlayerLoc + FVector(
        FMath::RandRange(-500.f, 500.f),
        FMath::RandRange(-500.f, 500.f),
        1000.f
    );

    //운석 스폰
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();

    GetWorld()->SpawnActor<AActor>(MeteorClass, SpawnLoc, FRotator::ZeroRotator, SpawnParams);
}
