#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class RELIKEMULTIPLAYER_API UPlayerHUDWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    // Bind these to your widgets
    UPROPERTY(meta = (BindWidget))
    UProgressBar* HealthBar;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* StaminaBar;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* HealthText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* StaminaText;

public:
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdateHealth(float HealthPercent);

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdateStamina(float StaminaPercent);
};