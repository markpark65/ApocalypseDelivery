#include "LookFreezeItem.h"
#include "Drone.h"

ALookFreezeItem::ALookFreezeItem()
{
	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(DefaultRoot);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ItemType = "ALookFreezeItem";
}

void ALookFreezeItem::ApplyEffect(ADrone* Drone)
{
	if (Drone) Drone->SetLookFreeze(5.0f);
	ShowPickupUI(Drone);
	PlayOverlapEffects();
	Super::ApplyEffect(Drone);
	Destroy();
}
