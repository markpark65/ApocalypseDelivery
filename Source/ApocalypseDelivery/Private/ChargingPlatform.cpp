#include "ChargingPlatform.h"
#include "Drone.h"
#include "Components/BoxComponent.h"
#include "ApocalypseHUD.h"
#include "ApocalypseGameMode.h"
#include "Kismet/GameplayStatics.h"

AChargingPlatform::AChargingPlatform()
{
	PrimaryActorTick.bCanEverTick = true;
    ChargingZone = CreateDefaultSubobject<UBoxComponent>(TEXT("ChargingZone"));
    SetRootComponent(ChargingZone);
    ChargingZone->SetBoxExtent(FVector(200.f, 200.f, 100.f));

    PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
    PlatformMesh->SetupAttachment(RootComponent);
}

void AChargingPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    TArray<AActor*> OverlappingActors;
    GetOverlappingActors(OverlappingActors, ADrone::StaticClass());

    for (AActor* Actor : OverlappingActors)
    {
        if (ADrone* Drone = Cast<ADrone>(Actor))
        {
            //Drone->AddBattery(RecoverBattery * DeltaTime);
        }
    }
}

