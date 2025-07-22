#include "StaminaComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

UStaminaComponent::UStaminaComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true);
    
    // Initialize default values
    CurrentStamina = MaxStamina;
    CurrentStaminaState = EStaminaState::Normal;
    bCanRegenerate = true;
}

void UStaminaComponent::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("StaminaComponent BeginPlay - Role: %d"), (int32)GetOwnerRole());
    
    // Initialize stamina on server
    if (GetOwnerRole() == ROLE_Authority)
    {
        CurrentStamina = MaxStamina;
        CurrentStaminaState = EStaminaState::Normal;
        UE_LOG(LogTemp, Warning, TEXT("StaminaComponent: Initialized on Authority - Stamina: %f"), CurrentStamina);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("StaminaComponent: Client initialization - waiting for replication"));
    }
}

void UStaminaComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Only process stamina changes on server
    if (GetOwnerRole() < ROLE_Authority) return;

    if (bIsSprinting || bIsRunning)
    {
        HandleStaminaDepletion(DeltaTime);
    }
    else if (bCanRegenerate)
    {
        HandleStaminaRegeneration(DeltaTime);
    }
}

void UStaminaComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UStaminaComponent, CurrentStamina);
    DOREPLIFETIME(UStaminaComponent, CurrentStaminaState);
}

void UStaminaComponent::OnRep_Stamina()
{
    OnStaminaChanged.Broadcast(CurrentStamina);
}

void UStaminaComponent::OnRep_StaminaState()
{
    OnStaminaStateChanged.Broadcast(CurrentStaminaState);
    
    if (CurrentStaminaState == EStaminaState::Exhausted)
    {
        ApplyExhaustionEffects();
    }
}

void UStaminaComponent::UpdateStaminaState()
{
    EStaminaState OldState = CurrentStaminaState;

    if (CurrentStamina <= 0)
    {
        CurrentStaminaState = EStaminaState::Exhausted;
        bIsExhausted = true;
    }
    else if (CurrentStamina <= LowStaminaThreshold)
    {
        CurrentStaminaState = EStaminaState::Low;
    }
    else
    {
        CurrentStaminaState = EStaminaState::Normal;
    }

    if (OldState != CurrentStaminaState)
    {
        OnStaminaStateChanged.Broadcast(CurrentStaminaState);

        if (CurrentStaminaState == EStaminaState::Exhausted)
        {
            OnExhausted.Broadcast();
            ApplyExhaustionEffects();
            
            // Start exhaustion recovery delay
            bCanRegenerate = false;
            GetWorld()->GetTimerManager().SetTimer(
                ExhaustionRecoveryTimerHandle,
                this,
                &UStaminaComponent::EndExhaustionRecoveryDelay,
                ExhaustedRecoveryDelay,
                false
            );
        }
        else if (OldState == EStaminaState::Exhausted && CurrentStamina >= ExhaustedRecoveryThreshold)
        {
            bIsExhausted = false;
            OnRecovered.Broadcast();
            RemoveExhaustionEffects();
        }
    }
}

void UStaminaComponent::HandleStaminaRegeneration(float DeltaTime)
{
    if (CurrentStamina >= MaxStamina) return;

    float OldStamina = CurrentStamina;
    CurrentStamina = FMath::Clamp(CurrentStamina + (RecoveryRatePerSecond * DeltaTime), 0.0f, MaxStamina);

    if (CurrentStamina != OldStamina)
    {
        OnStaminaChanged.Broadcast(CurrentStamina);
        UpdateStaminaState();
    }
}

void UStaminaComponent::HandleStaminaDepletion(float DeltaTime)
{
    float DepletionRate = 0.0f;
    
    if (bIsSprinting)
    {
        DepletionRate = SprintingCostPerSecond;
    }
    else if (bIsRunning)
    {
        DepletionRate = RunningCostPerSecond;
    }

    if (DepletionRate > 0)
    {
        ConsumeStamina(DepletionRate * DeltaTime);
    }
}

void UStaminaComponent::ApplyExhaustionEffects()
{
    ACharacter* Owner = Cast<ACharacter>(GetOwner());
    if (!Owner) return;

    UCharacterMovementComponent* MovementComp = Owner->GetCharacterMovement();
    if (!MovementComp) return;

    // Force walk speed when exhausted
    MovementComp->MaxWalkSpeed = 300.0f; // Slower than normal walk
    
    // Stop sprinting
    if (bIsSprinting)
    {
        StopSprinting();
    }

    // Visual/Audio feedback
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("EXHAUSTED! Cannot sprint!"));
    }
}

void UStaminaComponent::RemoveExhaustionEffects()
{
    ACharacter* Owner = Cast<ACharacter>(GetOwner());
    if (!Owner) return;

    UCharacterMovementComponent* MovementComp = Owner->GetCharacterMovement();
    if (!MovementComp) return;

    // Restore normal walk speed
    MovementComp->MaxWalkSpeed = 500.0f;
}

bool UStaminaComponent::ConsumeStamina(float Amount)
{
    if (GetOwnerRole() < ROLE_Authority)
    {
        Server_ConsumeStamina(Amount);
        return CurrentStamina >= Amount; // Predictive check
    }

    if (Amount <= 0) return true;

    float OldStamina = CurrentStamina;
    CurrentStamina = FMath::Clamp(CurrentStamina - Amount, 0.0f, MaxStamina);

    if (CurrentStamina != OldStamina)
    {
        OnStaminaChanged.Broadcast(CurrentStamina);
        UpdateStaminaState();
        LastStaminaChangeTime = GetWorld()->GetTimeSeconds();
    }

    return CurrentStamina > 0;
}

void UStaminaComponent::StartSprinting()
{
    if (GetOwnerRole() < ROLE_Authority)
    {
        Server_SetSprintState(true);
    }

    if (!CanSprint()) return;

    bIsSprinting = true;
    bCanRegenerate = false;

    // Update character movement speed
    if (ACharacter* Owner = Cast<ACharacter>(GetOwner()))
    {
        Owner->GetCharacterMovement()->MaxWalkSpeed = 800.0f; // Sprint speed
    }
}

void UStaminaComponent::StopSprinting()
{
    if (GetOwnerRole() < ROLE_Authority)
    {
        Server_SetSprintState(false);
    }

    bIsSprinting = false;
    
    // Allow regeneration after a short delay
    if (!bIsRunning)
    {
        FTimerHandle RegenerationDelayHandle;
        GetWorld()->GetTimerManager().SetTimer(
            RegenerationDelayHandle,
            [this]() { bCanRegenerate = true; },
            1.0f,
            false
        );
    }

    // Update character movement speed
    if (ACharacter* Owner = Cast<ACharacter>(GetOwner()))
    {
        Owner->GetCharacterMovement()->MaxWalkSpeed = bIsRunning ? 600.0f : 500.0f;
    }
}

void UStaminaComponent::StartRunning()
{
    if (GetOwnerRole() < ROLE_Authority)
    {
        Server_SetRunState(true);
    }

    bIsRunning = true;
    bCanRegenerate = false;

    if (!bIsSprinting)
    {
        if (ACharacter* Owner = Cast<ACharacter>(GetOwner()))
        {
            Owner->GetCharacterMovement()->MaxWalkSpeed = 600.0f; // Run speed
        }
    }
}

void UStaminaComponent::StopRunning()
{
    if (GetOwnerRole() < ROLE_Authority)
    {
        Server_SetRunState(false);
    }

    bIsRunning = false;
    
    if (!bIsSprinting)
    {
        FTimerHandle RegenerationDelayHandle;
        GetWorld()->GetTimerManager().SetTimer(
            RegenerationDelayHandle,
            [this]() { bCanRegenerate = true; },
            0.5f,
            false
        );

        if (ACharacter* Owner = Cast<ACharacter>(GetOwner()))
        {
            Owner->GetCharacterMovement()->MaxWalkSpeed = 500.0f; // Normal walk speed
        }
    }
}

bool UStaminaComponent::PerformMeleeAttack()
{
    if (!CanPerformAction(MeleeCost)) return false;
    
    ConsumeStamina(MeleeCost);
    return true;
}

bool UStaminaComponent::PerformStruggle()
{
    if (!CanPerformAction(StruggleCost)) return false;
    
    ConsumeStamina(StruggleCost);
    return true;
}

void UStaminaComponent::EndExhaustionRecoveryDelay()
{
    bCanRegenerate = true;
}

// Server RPC implementations
void UStaminaComponent::Server_ConsumeStamina_Implementation(float Amount)
{
    ConsumeStamina(Amount);
}

void UStaminaComponent::Server_SetSprintState_Implementation(bool bSprinting)
{
    if (bSprinting)
    {
        StartSprinting();
    }
    else
    {
        StopSprinting();
    }
}

void UStaminaComponent::Server_SetRunState_Implementation(bool bRunning)
{
    if (bRunning)
    {
        StartRunning();
    }
    else
    {
        StopRunning();
    }
}
