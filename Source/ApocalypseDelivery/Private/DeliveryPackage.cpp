#include "DeliveryPackage.h"
#include "ApocalypseGameMode.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"

ADeliveryPackage::ADeliveryPackage()
{
    //PrimaryActorTick.bCanEverTick = true;
    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    RootComponent = MeshComp;
    Tags.Add(FName("Package"));

}

void ADeliveryPackage::BeginPlay()
{
	Super::BeginPlay();
	
}

