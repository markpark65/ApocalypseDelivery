#include "CureItem.h"
#include "Drone.h"

ACureItem::ACureItem() {
    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    ItemMesh->SetupAttachment(DefaultRoot);
    ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ItemType = "CureItem";
    PickupMessage = TEXT("[버프 아이템] 모든 디버프 효과 제거!!");
}

void ACureItem::ApplyEffect(ADrone* Drone) {
    if (Drone) {
        Drone->ClearAllDebuffs();
        Drone->ResetSpeed();
        PlayOverlapEffects();
        ShowPickupUI(Drone);
    }

    Destroy();
}
