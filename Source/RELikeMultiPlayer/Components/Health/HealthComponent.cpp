#include "HealthComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

UHealthComponent::UHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
    
    // Initialize default values
    CurrentHealth = MaxHealth;
    CurrentHealthState = EHealthState::Healthy;
    bIsDowned = false;
}

void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();
    
    	UE_LOG(LogTemp, Log, TEXT("HealthComponent BeginPlay - Role: %d"), (int32)GetOwnerRole());
    
    // Only set health on server
    if (GetOwnerRole() == ROLE_Authority)
    {
        CurrentHealth = MaxHealth;
        UpdateHealthState();
        		UE_LOG(LogTemp, Log, TEXT("HealthComponent: Initialized on Authority - Health: %f"), CurrentHealth);
    }
    else
    {
        		UE_LOG(LogTemp, Log, TEXT("HealthComponent: Client initialization - waiting for replication"));
    }
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UHealthComponent, CurrentHealth);
    DOREPLIFETIME(UHealthComponent, CurrentHealthState);
    DOREPLIFETIME(UHealthComponent, bIsDowned);
}

void UHealthComponent::OnRep_Health()
{
    OnHealthChanged.Broadcast(CurrentHealth);
}

void UHealthComponent::OnRep_HealthState()
{
    OnHealthStateChanged.Broadcast(CurrentHealthState);
    ApplyHealthStateEffects();
}

void UHealthComponent::UpdateHealthState()
{
    EHealthState OldState = CurrentHealthState;

    if (CurrentHealth <= 0)
    {
        if (!bIsDowned)
        {
            bIsDowned = true;
            CurrentHealthState = EHealthState::Downed;
            Multicast_OnDowned();
        }
        else
        {
            CurrentHealthState = EHealthState::Dead;
            Multicast_OnDied();
        }
    }
    else if (CurrentHealth >= 75.0f)
    {
        CurrentHealthState = EHealthState::Healthy;
    }
    else if (CurrentHealth >= 50.0f)
    {
        CurrentHealthState = EHealthState::Injured;
    }
    else if (CurrentHealth >= 25.0f)
    {
        CurrentHealthState = EHealthState::Wounded;
    }
    else
    {
        CurrentHealthState = EHealthState::Critical;
    }

    if (OldState != CurrentHealthState)
    {
        OnHealthStateChanged.Broadcast(CurrentHealthState);
        ApplyHealthStateEffects();
    }
}

void UHealthComponent::ApplyHealthStateEffects()
{
    ACharacter* Owner = Cast<ACharacter>(GetOwner());
    if (!Owner) return;

    UCharacterMovementComponent* MovementComp = Owner->GetCharacterMovement();
    if (!MovementComp) return;

    // Apply movement speed modifier
    float SpeedModifier = MovementSpeedModifiers.Contains(CurrentHealthState) 
        ? MovementSpeedModifiers[CurrentHealthState] 
        : 1.0f;

    MovementComp->MaxWalkSpeed = 500.0f * SpeedModifier;

    // Handle special states
    switch (CurrentHealthState)
    {
    case EHealthState::Downed:
        MovementComp->DisableMovement();
        // TODO: Play downed animation
        break;
    case EHealthState::Dead:
        MovementComp->DisableMovement();
        Owner->SetLifeSpan(10.0f); // Destroy after 10 seconds
        // TODO: Play death animation
        break;
    default:
        if (MovementComp->MovementMode == MOVE_None)
        {
            MovementComp->SetMovementMode(MOVE_Walking);
        }
        break;
    }
}

void UHealthComponent::TakeDamage(float DamageAmount, AActor* DamageCauser)
{
    if (GetOwnerRole() < ROLE_Authority)
    {
        Server_TakeDamage(DamageAmount, DamageCauser);
        return;
    }

    if (CurrentHealthState == EHealthState::Dead) return;

    float OldHealth = CurrentHealth;
    CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);

    if (CurrentHealth != OldHealth)
    {
        OnHealthChanged.Broadcast(CurrentHealth);
        UpdateHealthState();

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-7, 2.0f, FColor::Red, 
                FString::Printf(TEXT("Damage: %.1f, Health: %.1f"), DamageAmount, CurrentHealth));
        }
    }
}

void UHealthComponent::Heal(float HealAmount)
{
    if (GetOwnerRole() < ROLE_Authority)
    {
        Server_Heal(HealAmount);
        return;
    }

    if (CurrentHealthState == EHealthState::Dead) return;

    float OldHealth = CurrentHealth;
    CurrentHealth = FMath::Clamp(CurrentHealth + HealAmount, 0.0f, MaxHealth);

    if (CurrentHealth != OldHealth)
    {
        OnHealthChanged.Broadcast(CurrentHealth);
        UpdateHealthState();

        if (bIsDowned && CurrentHealth > 0)
        {
            bIsDowned = false;
        }
    }
}

void UHealthComponent::StartRevival(APawn* Reviver)
{
    if (GetOwnerRole() < ROLE_Authority)
    {
        Server_StartRevival(Reviver);
        return;
    }

    if (!bIsDowned || CurrentHealthState == EHealthState::Dead) return;

    CurrentReviver = Reviver;
    
    GetWorld()->GetTimerManager().SetTimer(
        RevivalTimerHandle,
        this,
        &UHealthComponent::CompleteRevival,
        RevivalTime,
        false
    );

    // TODO: Start revival UI/animation
}

void UHealthComponent::CompleteRevival()
{
    if (GetOwnerRole() < ROLE_Authority) return;

    if (bIsDowned && CurrentReviver.IsValid())
    {
        Heal(RevivalHealthAmount);
        bIsDowned = false;
        
        // Re-enable movement
        if (ACharacter* Owner = Cast<ACharacter>(GetOwner()))
        {
            Owner->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        }
    }

    CurrentReviver = nullptr;
}

void UHealthComponent::CancelRevival()
{
    if (GetOwnerRole() == ROLE_Authority)
    {
        GetWorld()->GetTimerManager().ClearTimer(RevivalTimerHandle);
        CurrentReviver = nullptr;
    }
}

// Server RPC implementations
void UHealthComponent::Server_TakeDamage_Implementation(float DamageAmount, AActor* DamageCauser)
{
    TakeDamage(DamageAmount, DamageCauser);
}

void UHealthComponent::Server_Heal_Implementation(float HealAmount)
{
    Heal(HealAmount);
}

void UHealthComponent::Server_StartRevival_Implementation(APawn* Reviver)
{
    StartRevival(Reviver);
}

void UHealthComponent::Multicast_OnDowned_Implementation()
{
    OnPlayerDowned.Broadcast();
}

void UHealthComponent::Multicast_OnDied_Implementation()
{
    OnPlayerDied.Broadcast();
}
