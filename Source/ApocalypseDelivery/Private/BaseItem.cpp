#include "BaseItem.h"
#include "Drone.h"
#include "Components/SphereComponent.h"
#include "Blueprint/UserWidget.h"
#include "ItemMessageWidget.h"
#include "Components/StaticMeshComponent.h"

ABaseItem::ABaseItem()
{
    PrimaryActorTick.bCanEverTick = true;

    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
    RootComponent = CollisionComp;
    CollisionComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

    DefaultRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRoot"));
    DefaultRoot->SetupAttachment(RootComponent);

    ItemLabelWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("ItemLabelWidget"));
    ItemLabelWidget->SetupAttachment(RootComponent);

    // 중요 설정
    ItemLabelWidget->SetWidgetSpace(EWidgetSpace::Screen);
    ItemLabelWidget->SetDrawAtDesiredSize(true);
}
void ABaseItem::BeginPlay()
{
    Super::BeginPlay();

    if (ItemLabelWidget)
    {
        UUserWidget* UserWidget = ItemLabelWidget->GetUserWidgetObject();
        if (UserWidget)
        {
            UTextBlock* NameText = Cast<UTextBlock>(UserWidget->GetWidgetFromName(TEXT("ItemNameText")));
            if (NameText)
            {
                NameText->SetText(FText::FromString(ItemDisplayName));
            }
        }
    }
}
// BaseItem.cpp
void ABaseItem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

    if (!PlayerPawn || !ItemLabelWidget) return;

    float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), GetActorLocation());

    if (Distance > DisplayDistance)
    {
        ItemLabelWidget->SetVisibility(false);
        return;
    }

    ItemLabelWidget->SetVisibility(true);

    UUserWidget* UserWidget = ItemLabelWidget->GetUserWidgetObject();
    if (UserWidget)
    {
        float TargetOpacity = FMath::Clamp(1.0f - (Distance / DisplayDistance), 0.0f, 1.0f);
        UserWidget->SetRenderOpacity(TargetOpacity);
    }
}
void ABaseItem::ShowPickupUI(ADrone* Drone)
{
    // 드론이 없거나, 위젯 클래스가 설정되지 않았거나, 메시지가 비어있으면 실행하지 않음
    if (!Drone || !ItemMessageWidgetClass || PickupMessage.IsEmpty()) return;

    APlayerController* PC = Cast<APlayerController>(Drone->GetController());
    if (PC)
    {
        // 위젯 생성
        UUserWidget* RawWidget = CreateWidget<UUserWidget>(PC, ItemMessageWidgetClass);
        UItemMessageWidget* ItemWidget = Cast<UItemMessageWidget>(RawWidget);

        if (ItemWidget)
        {
            // 메시지 설정 및 화면에 추가
            ItemWidget->SetMessage(PickupMessage);
            ItemWidget->AddToViewport();

            UE_LOG(LogTemp, Warning, TEXT("ItemCollided!"));
            // 2초 뒤에 위젯을 제거하는 타이머 설정
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, [ItemWidget]() {
                if (ItemWidget) ItemWidget->RemoveFromParent();
                }, 2.0f, false);
        }
    }
}
FName ABaseItem::GetItemType() const
{
    return ItemType;
}
