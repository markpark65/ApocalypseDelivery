#include "SlowingItem.h"
#include "Drone.h"

ASlowingItem::ASlowingItem()
{
	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(DefaultRoot);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ItemType = "ASlowingItem";
}

void ASlowingItem::ApplyEffect(ADrone* Drone)
{
	if (Drone) Drone->SetTemporarySpeed(0.5f, 7.0f);
	ShowPickupUI(Drone);
	PlayOverlapEffects();
	Super::ApplyEffect(Drone);
	Destroy();
}

