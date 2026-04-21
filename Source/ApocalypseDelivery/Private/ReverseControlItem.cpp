#include "ReverseControlItem.h"
#include "Drone.h"

AReverseControlItem::AReverseControlItem()
{
	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(DefaultRoot);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ItemType = "ReverseControlItem";
}

void AReverseControlItem::ApplyEffect(ADrone* Drone)
{
	Drone->SetReverseControl(10.0f);
	ShowPickupUI(Drone);
	PlayOverlapEffects();
	Super::ApplyEffect(Drone);
	Destroy();
}
