#include "DeliveryPlatform.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Drone.h"
#include "ApocalypseHUD.h"
#include "ApocalypseGameMode.h"
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
    //현재 배달 지정 플랫폼만 보이기에 사용안됨
    /*if (!TargetIndicatorMesh->IsVisible())
    {
        if (OtherActor && OtherActor->ActorHasTag("Package"))
        {
            if (auto* GM = Cast<AApocalypseGameMode>(GetWorld()->GetAuthGameMode()))
            {
                if (GM->CurrentHUD)
                {
                    GM->CurrentHUD->ShowWrongDeliveryUI();
                }
            }
            UE_LOG(LogTemp, Warning, TEXT("Not the target platform!"));
        }
        return;
    }*/

    if (OtherActor->ActorHasTag("Package"))
    {
        TArray<AActor*> FoundDrones;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADrone::StaticClass(), FoundDrones);

        for (AActor* Actor : FoundDrones)
        {
            if (ADrone* Drone = Cast<ADrone>(Actor))
            {
                if (Drone->GetAttachedPackage() == OtherActor)
                {
                    Drone->SetAttachedPackage(nullptr);

                    if (AApocalypseGameMode* GM = Cast<AApocalypseGameMode>(GetWorld()->GetAuthGameMode()))
                    {
                        if (GM->CurrentHUD) GM->CurrentHUD->SetInteractionPrompt(false, TEXT(""));
                    }
                }
            }
        }

        // 상자 파괴 및 효과음
        //OtherActor->Destroy();
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

void ADeliveryPlatform::SetIsTarget(bool bIsTarget)
{
    /*
    if (TargetIndicatorMesh)
    {
        TargetIndicatorMesh->SetVisibility(bIsTarget);
    }*/
}