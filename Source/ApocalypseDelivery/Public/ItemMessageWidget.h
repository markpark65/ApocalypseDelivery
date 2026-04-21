#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "ItemMessageWidget.generated.h"

UCLASS()
class APOCALYPSEDELIVERY_API UItemMessageWidget : public UUserWidget
{
	GENERATED_BODY()
public:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* MessageText;

    void SetMessage(FString Message);
};
