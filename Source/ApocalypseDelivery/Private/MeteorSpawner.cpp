#include "MeteorSpawner.h"
#include "Meteor.h"
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

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();

    int32 RandomChance = FMath::RandRange(0, 100);

    if (RandomChance < 70)
    {
        // 플레이어 조준 + 오차 범위
        FVector ForwardOffset = PlayerPawn->GetActorForwardVector() * 800.f;
        FVector SpawnLoc = PlayerPawn->GetActorLocation() + ForwardOffset + FVector(0, 0, 800.f);

        AMeteor* NewMeteor = GetWorld()->SpawnActor<AMeteor>(MeteorClass, SpawnLoc, FRotator::ZeroRotator, SpawnParams);
        if (NewMeteor)
        {
            // 드론 주변 3미터 오차 범위 내 조준
            FVector RandomOffset = FVector(FMath::RandRange(-300.f, 300.f), FMath::RandRange(-300.f, 300.f), 0.f);
            NewMeteor->SetMeteorDirection(PlayerPawn->GetActorLocation() + RandomOffset);
        }
    }
    else
    {
        // 플레이어 근처 하늘에서 수직 낙하
        // 드론 주변 5미터 반경 랜덤 위치에서 수직으로 떨어짐
        FVector RandomSpawnLoc = PlayerPawn->GetActorLocation() + FVector(
            FMath::RandRange(-500.f, 500.f),
            FMath::RandRange(-500.f, 500.f),
            1200.f
        );

        AMeteor* NewMeteor = GetWorld()->SpawnActor<AMeteor>(MeteorClass, RandomSpawnLoc, FRotator::ZeroRotator, SpawnParams);
        if (NewMeteor)
        {
            NewMeteor->SetMeteorSpeed(1200.f);
        }
    }
}
