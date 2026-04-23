#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ADInteractable.h"
#include "Components/WidgetComponent.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "BaseItem.generated.h"

UCLASS()
class APOCALYPSEDELIVERY_API ABaseItem : public AActor, public IADInteractable
{
	GENERATED_BODY()
	
public:	
	ABaseItem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* DefaultRoot;
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class USphereComponent* CollisionComp;

    virtual void ApplyEffect(class ADrone* Drone) override {}

    virtual FName GetItemType() const override;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FName ItemType;

    //UI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | UI")
    FString PickupMessage;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | UI")
    TSubclassOf<class UUserWidget> ItemMessageWidgetClass;
    void ShowPickupUI(ADrone* Drone);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
    class UWidgetComponent* ItemLabelWidget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FString ItemDisplayName;

    //ItemLabel 가시성이 안좋은 이유 중 획득 직전이 되어서야 Label이 보이는 문제도 있다고 생각해서 기존 900.0f에서 2000.0f로 수정.
    UPROPERTY(EditAnywhere, Category = "UI")
    float DisplayDistance = 2000.0f;

    //이펙트
    UPROPERTY(EditAnywhere, Category = "Effects")
    class UParticleSystem* OverlapParticle;
    UPROPERTY(EditAnywhere, Category = "Effects")
    class USoundBase* OverlapSound;

    void PlayOverlapEffects() {
        if (OverlapParticle) UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), OverlapParticle, GetActorLocation());
        if (OverlapSound) UGameplayStatics::PlaySoundAtLocation(this, OverlapSound, GetActorLocation());
    }
};
