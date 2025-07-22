// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "HealthComponent.generated.h"

UENUM(BlueprintType)
enum class EHealthState : uint8
{
    Healthy      UMETA(DisplayName = "Healthy"),      // 100-75 HP
    Injured      UMETA(DisplayName = "Injured"),      // 74-50 HP
    Wounded      UMETA(DisplayName = "Wounded"),      // 49-25 HP
    Critical     UMETA(DisplayName = "Critical"),     // 24-1 HP
    Downed       UMETA(DisplayName = "Downed"),       // 0 HP but revivable
    Dead         UMETA(DisplayName = "Dead")          // Permanently dead
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthStateChanged, EHealthState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDowned);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDied);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RELIKEMULTIPLAYER_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

	protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Health Properties
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
    float MaxHealth = 100.0f;

    UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = "Health")
    float CurrentHealth;

    UPROPERTY(ReplicatedUsing = OnRep_HealthState, BlueprintReadOnly, Category = "Health")
    EHealthState CurrentHealthState;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Health")
    bool bIsDowned = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
    float DownedHealthThreshold = 25.0f;

    // Revival Properties
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Revival")
    float RevivalTime = 10.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Revival")
    float RevivalHealthAmount = 25.0f;

    // Movement speed modifiers per state
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health Effects")
    TMap<EHealthState, float> MovementSpeedModifiers = {
        {EHealthState::Healthy, 1.0f},
        {EHealthState::Injured, 0.9f},
        {EHealthState::Wounded, 0.7f},
        {EHealthState::Critical, 0.5f},
        {EHealthState::Downed, 0.0f},
        {EHealthState::Dead, 0.0f}
    };

    // Replication functions
    UFUNCTION()
    void OnRep_Health();

    UFUNCTION()
    void OnRep_HealthState();

    // Internal functions
    void UpdateHealthState();
    void ApplyHealthStateEffects();

public:
    // Public functions
    UFUNCTION(BlueprintCallable, Category = "Health")
    void TakeDamage(float DamageAmount, AActor* DamageCauser = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Health")
    void Heal(float HealAmount);

    UFUNCTION(BlueprintCallable, Category = "Health")
    float GetHealthPercentage() const { return CurrentHealth / MaxHealth; }

    UFUNCTION(BlueprintCallable, Category = "Health")
    EHealthState GetHealthState() const { return CurrentHealthState; }

    UFUNCTION(BlueprintCallable, Category = "Health")
    bool IsAlive() const { return CurrentHealthState != EHealthState::Dead; }

    UFUNCTION(BlueprintCallable, Category = "Health")
    bool IsDowned() const { return bIsDowned; }

    // Revival functions
    UFUNCTION(BlueprintCallable, Category = "Revival", meta = (CallInEditor = "true"))
    void StartRevival(APawn* Reviver);

    UFUNCTION(BlueprintCallable, Category = "Revival")
    void CompleteRevival();

    UFUNCTION(BlueprintCallable, Category = "Revival")
    void CancelRevival();

    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Health")
    FOnHealthChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "Health")
    FOnHealthStateChanged OnHealthStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Health")
    FOnPlayerDowned OnPlayerDowned;

    UPROPERTY(BlueprintAssignable, Category = "Health")
    FOnPlayerDied OnPlayerDied;

private:
    FTimerHandle RevivalTimerHandle;
    TWeakObjectPtr<APawn> CurrentReviver;

    UFUNCTION(Server, Reliable)
    void Server_TakeDamage(float DamageAmount, AActor* DamageCauser);

    UFUNCTION(Server, Reliable)
    void Server_Heal(float HealAmount);

    UFUNCTION(Server, Reliable)
    void Server_StartRevival(APawn* Reviver);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_OnDowned();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_OnDied();
};