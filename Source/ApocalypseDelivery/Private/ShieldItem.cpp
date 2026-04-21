#include "ShieldItem.h"
#include "Drone.h"

AShieldItem::AShieldItem() {
    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    ItemMesh->SetupAttachment(DefaultRoot);
    ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ItemType = "AShieldItem";
    PickupMessage = TEXT("[버프 아이템] 베리어 생성!!");
}

void AShieldItem::ApplyEffect(ADrone* Drone) {
    if (Drone)
    {
        Drone->SetShield(true);
        PlayOverlapEffects();
        ShowPickupUI(Drone);
    }
    Destroy();
}
