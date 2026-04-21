#include "AirMine.h"
#include "Drone.h"
#include "ApocalypseGameMode.h"

AAirMine::AAirMine()
{
    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    ItemMesh->SetupAttachment(DefaultRoot);
    ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ItemType = "Mine";
}

void AAirMine::ApplyEffect(ADrone* Drone)
{
    UE_LOG(LogTemp, Warning, TEXT("AIRMINE EXPLODED!!!"));
    if (!Drone) return;

    // 드론 상태에 따른 효과 적용
    if (Drone->bHasShield)
    {
        // 실드가 있으면 실드만 제거 (배리어 파괴)
        Drone->SetShield(false);
        UE_LOG(LogTemp, Warning, TEXT("Mine Exploded! Shield Absorbed Damage."));
    }
    else
    {
        // 실드가 없으면 즉시 게임 오버 처리
        UE_LOG(LogTemp, Error, TEXT("Mine Exploded! Drone Destroyed."));

        if (AApocalypseGameMode* GM = Cast<AApocalypseGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
        {
            Drone->HandleGameOver();
        }
    }

    PlayOverlapEffects();
    ShowPickupUI(Drone);

    Super::ApplyEffect(Drone);
    Destroy();
}