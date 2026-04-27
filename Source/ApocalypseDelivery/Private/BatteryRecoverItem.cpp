#include "BatteryRecoverItem.h"
#include "Drone.h"

ABatteryRecoverItem::ABatteryRecoverItem() {
    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    ItemMesh->SetupAttachment(DefaultRoot);
    ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ItemType = "ABatteryRecoverItem";
    PickupMessage = TEXT("[버프 아이템] 배터리가 회복 됐습니다!!");
}

void ABatteryRecoverItem::ApplyEffect(ADrone* Drone) {
    if (Drone)
    {
        //Drone->AddBattery(30.0f); // 30% 회복
        PlayOverlapEffects();
        ShowPickupUI(Drone);
    }
    Destroy();
}
