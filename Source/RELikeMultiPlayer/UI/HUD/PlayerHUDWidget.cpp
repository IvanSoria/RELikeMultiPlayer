// PlayerHUDWidget.cpp
#include "PlayerHUDWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "../../Player/Character/RELikeMultiPlayerCharacter.h"
#include "../../Components/Health/HealthComponent.h"
#include "../../Components/Stamina/StaminaComponent.h"

void UPlayerHUDWidget::NativeDestruct()
{
    // Clean up bindings
    if (HealthComponent)
    {
        HealthComponent->OnHealthChanged.RemoveDynamic(this, &UPlayerHUDWidget::OnHealthChanged);
    }
    
    if (StaminaComponent)
    {
        StaminaComponent->OnStaminaChanged.RemoveDynamic(this, &UPlayerHUDWidget::OnStaminaChanged);
    }
    
    Super::NativeDestruct();
}

void UPlayerHUDWidget::SetupPlayerComponents(ARELikeMultiPlayerCharacter* Character)
{
    if (!Character) return;
    
    // Bind to health component
    HealthComponent = Character->GetHealthComponent();
    if (HealthComponent)
    {
        HealthComponent->OnHealthChanged.AddDynamic(this, &UPlayerHUDWidget::OnHealthChanged);
        // Initial update
        UpdateHealth(HealthComponent->GetHealthPercentage());
    }
    
    // Bind to stamina component
    StaminaComponent = Character->GetStaminaComponent();
    if (StaminaComponent)
    {
        StaminaComponent->OnStaminaChanged.AddDynamic(this, &UPlayerHUDWidget::OnStaminaChanged);
        // Initial update
        UpdateStamina(StaminaComponent->GetStaminaPercentage());
    }
}

void UPlayerHUDWidget::OnHealthChanged(float NewHealth)
{
    if (HealthComponent)
    {
        UpdateHealth(HealthComponent->GetHealthPercentage());
    }
}

void UPlayerHUDWidget::OnStaminaChanged(float NewStamina)
{
    if (StaminaComponent)
    {
        UpdateStamina(StaminaComponent->GetStaminaPercentage());
    }
}

void UPlayerHUDWidget::UpdateHealth(float HealthPercent)
{
    if (HealthBar)
    {
        HealthBar->SetPercent(HealthPercent);
    }

    if (HealthText)
    {
        // Display as percentage
        int32 HealthPercentage = FMath::RoundToInt(HealthPercent * 100.0f);
        HealthText->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), HealthPercentage)));

        // Optional: Change color based on health
        FSlateColor HealthColor;
        if (HealthPercent > 0.75f)
        {
            HealthColor = FSlateColor(FLinearColor::Green);
        }
        else if (HealthPercent > 0.25f)
        {
            HealthColor = FSlateColor(FLinearColor::Yellow);
        }
        else
        {
            HealthColor = FSlateColor(FLinearColor::Red);
        }
        
        HealthText->SetColorAndOpacity(HealthColor);
    }
}

void UPlayerHUDWidget::UpdateStamina(float StaminaPercent)
{
    if (StaminaBar)
    {
        StaminaBar->SetPercent(StaminaPercent);
    }

    if (StaminaText)
    {
        // Display as percentage
        int32 StaminaPercentage = FMath::RoundToInt(StaminaPercent * 100.0f);
        StaminaText->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), StaminaPercentage)));

        // Optional: Change color when exhausted
        if (StaminaPercent <= 0.0f)
        {
            StaminaText->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
        }
        else if (StaminaPercent < 0.3f)
        {
            StaminaText->SetColorAndOpacity(FSlateColor(FLinearColor::Yellow));
        }
        else
        {
            StaminaText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
        }
    }
}