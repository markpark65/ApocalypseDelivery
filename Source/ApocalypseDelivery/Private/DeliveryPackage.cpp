#include "DeliveryPackage.h"
#include "ApocalypseGameMode.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"

ADeliveryPackage::ADeliveryPackage()
{
    PrimaryActorTick.bCanEverTick = true;
    CollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComp"));
    RootComponent = CollisionComp;
    CollisionComp->SetBoxExtent(FVector(50.f, 50.f, 50.f));
    CollisionComp->SetCollisionProfileName(TEXT("PhysicsActor"));

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(RootComponent);

    Tags.Add(FName("Package"));

}

void ADeliveryPackage::BeginPlay()
{
	Super::BeginPlay();
	
}

void ADeliveryPackage::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (GetActorLocation().Z < -500.0f)
    {
        if (AApocalypseGameMode* GM = Cast<AApocalypseGameMode>(GetWorld()->GetAuthGameMode()))
        {
            GM->EndGame(false);
        }
        Destroy();
    }
}


