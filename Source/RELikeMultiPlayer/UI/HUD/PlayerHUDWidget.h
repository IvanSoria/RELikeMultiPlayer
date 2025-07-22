#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;
class ARELikeMultiPlayerCharacter;
class UHealthComponent;
class UStaminaComponent;

UCLASS()
class RELIKEMULTIPLAYER_API UPlayerHUDWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeDestruct() override;

    // Bind these to your widgets
    UPROPERTY(meta = (BindWidget))
    UProgressBar* HealthBar;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* StaminaBar;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* HealthText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* StaminaText;

    // Component references
    UPROPERTY()
    UHealthComponent* HealthComponent;

    UPROPERTY()
    UStaminaComponent* StaminaComponent;

    // Event handlers
    UFUNCTION()
    void OnHealthChanged(float NewHealth);

    UFUNCTION()
    void OnStaminaChanged(float NewStamina);

public:
    // Setup function to bind to player character components
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void SetupPlayerComponents(ARELikeMultiPlayerCharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdateHealth(float HealthPercent);

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdateStamina(float StaminaPercent);
};