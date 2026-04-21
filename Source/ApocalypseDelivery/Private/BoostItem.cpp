#include "BoostItem.h"
#include "Drone.h"

ABoostItem::ABoostItem() {
    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    ItemMesh->SetupAttachment(DefaultRoot);
    ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ItemType = "ABoostItem";
    PickupMessage = TEXT("[버프 아이템] 부스터 획득!!");
}

void ABoostItem::ApplyEffect(ADrone* Drone) {
    if (Drone) {
        Drone->SetTemporarySpeed(1.5f, 5.0f);
        PlayOverlapEffects();
        ShowPickupUI(Drone);
    }
    Destroy();
}
