#include "DeliveryPlatform.h"
#include "Drone.h"
#include "ApocalypseHUD.h"
#include "ApocalypseGameMode.h"
#include "DeliveryPackage.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

ADeliveryPlatform::ADeliveryPlatform()
{
	PrimaryActorTick.bCanEverTick = true;

    DeliveryZone = CreateDefaultSubobject<UBoxComponent>(TEXT("DeliveryZone"));
    SetRootComponent(DeliveryZone);
    DeliveryZone->SetBoxExtent(FVector(200.f, 200.f, 100.f));

    PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
    PlatformMesh->SetupAttachment(RootComponent);

    /*
    TargetIndicatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TargetIndicatorMesh"));
    TargetIndicatorMesh->SetupAttachment(RootComponent);
    TargetIndicatorMesh->SetVisibility(false);*/

    InitialRelativeZ = 150.0f;
}

void ADeliveryPlatform::BeginPlay()
{
	Super::BeginPlay();

    /*
    if (TargetIndicatorMesh)
    {
        InitialRelativeZ = TargetIndicatorMesh->GetRelativeLocation().Z;
    }*/
	
}
void ADeliveryPlatform::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    /*
    if (TargetIndicatorMesh && TargetIndicatorMesh->IsVisible())
    {
        // 회전 로직
        FRotator NewRotation = TargetIndicatorMesh->GetRelativeRotation();
        NewRotation.Yaw += RotationSpeed * DeltaTime;
        TargetIndicatorMesh->SetRelativeRotation(NewRotation);

        // 위아래 이동 로직
        float RunningTime = GetWorld()->GetTimeSeconds();
        float NewZ = InitialRelativeZ + (FMath::Sin(RunningTime * BobbingSpeed) * BobbingAmount);

        FVector NewLocation = TargetIndicatorMesh->GetRelativeLocation();
        NewLocation.Z = NewZ;
        TargetIndicatorMesh->SetRelativeLocation(NewLocation);
    }*/
}
//택배 배송 완료 로직
void ADeliveryPlatform::NotifyActorBeginOverlap(AActor* OtherActor)
{
    Super::NotifyActorBeginOverlap(OtherActor);
    if (!OtherActor) return;
    if (bIsUsed)
    {
        return;
    }

    if (OtherActor->ActorHasTag("Package"))
    {
        TArray<AActor*> FoundDrones;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADrone::StaticClass(), FoundDrones);

        for (AActor* Actor : FoundDrones)
        {
            if (ADrone* Drone = Cast<ADrone>(Actor))
            {
                if (OtherActor->IsA(ADeliveryPackage::StaticClass()))
                {
                    if (AApocalypseGameMode* GM = Cast<AApocalypseGameMode>(GetWorld()->GetAuthGameMode()))
                    {
                        if (GM->CurrentHUD) GM->CurrentHUD->SetInteractionPrompt(false, TEXT(""));
                    }
                }
            }
        }

        // 상자 배달 효과음
        if (SuccessSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, SuccessSound, GetActorLocation());
        }

        // 게임모드에 알림
        if (AApocalypseGameMode* GM = Cast<AApocalypseGameMode>(GetWorld()->GetAuthGameMode()))
        {
            GM->OnPackageDelivered(this);
        }
    }
}

void ADeliveryPlatform::MarkAsUsed()
{
    bIsUsed = true;

    if (PlatformMesh)
    {
        UMaterialInstanceDynamic* DynMaterial = PlatformMesh->CreateAndSetMaterialInstanceDynamic(0);
        if (DynMaterial)
        {
            DynMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor::Red);
        }
    }
}