#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ResultGameMode.generated.h"

UCLASS()
class APOCALYPSEDELIVERY_API AResultGameMode : public AGameModeBase
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;

    // 에디터에서 할당할 결과 위젯 클래스
    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<class UUserWidget> ResultWidgetClass;
};