// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "StaminaComponent.generated.h"

UENUM(BlueprintType)
enum class EStaminaState : uint8
{
    Normal       UMETA(DisplayName = "Normal"),
    Low          UMETA(DisplayName = "Low"),
    Exhausted    UMETA(DisplayName = "Exhausted")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStaminaChanged, float, NewStamina);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStaminaStateChanged, EStaminaState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExhausted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRecovered);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RELIKEMULTIPLAYER_API UStaminaComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStaminaComponent();

	protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Stamina Properties
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina")
    float MaxStamina = 100.0f;

    UPROPERTY(ReplicatedUsing = OnRep_Stamina, BlueprintReadOnly, Category = "Stamina")
    float CurrentStamina;

    UPROPERTY(ReplicatedUsing = OnRep_StaminaState, BlueprintReadOnly, Category = "Stamina")
    EStaminaState CurrentStaminaState;

    // Stamina Costs
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina|Costs")
    float RunningCostPerSecond = 2.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina|Costs")
    float SprintingCostPerSecond = 5.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina|Costs")
    float MeleeCost = 15.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina|Costs")
    float StruggleCost = 10.0f;

    // Recovery Settings
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina|Recovery")
    float RecoveryRatePerSecond = 3.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina|Recovery")
    float ExhaustedRecoveryDelay = 5.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina|Recovery")
    float ExhaustedRecoveryThreshold = 25.0f;

    // State Thresholds
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina|Thresholds")
    float LowStaminaThreshold = 30.0f;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Stamina|State")
    bool bIsSprinting = false;

    UPROPERTY(BlueprintReadOnly, Category = "Stamina|State")
    bool bIsRunning = false;

    UPROPERTY(BlueprintReadOnly, Category = "Stamina|State")
    bool bIsExhausted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Stamina|State")
    bool bCanRegenerate = true;

    // Replication functions
    UFUNCTION()
    void OnRep_Stamina();

    UFUNCTION()
    void OnRep_StaminaState();

    // Internal functions
    void UpdateStaminaState();
    void HandleStaminaRegeneration(float DeltaTime);
    void HandleStaminaDepletion(float DeltaTime);
    void ApplyExhaustionEffects();
    void RemoveExhaustionEffects();

public:
    // Public Functions
    UFUNCTION(BlueprintCallable, Category = "Stamina")
    bool ConsumeStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    void StartSprinting();

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    void StopSprinting();

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    void StartRunning();

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    void StopRunning();

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    bool CanSprint() const { return CurrentStamina > 0 && !bIsExhausted; }

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    bool CanPerformAction(float RequiredStamina) const { return CurrentStamina >= RequiredStamina && !bIsExhausted; }

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    float GetStaminaPercentage() const { return CurrentStamina / MaxStamina; }

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    EStaminaState GetStaminaState() const { return CurrentStaminaState; }

    // Combat Functions
    UFUNCTION(BlueprintCallable, Category = "Stamina|Combat")
    bool PerformMeleeAttack();

    UFUNCTION(BlueprintCallable, Category = "Stamina|Combat")
    bool PerformStruggle();

    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Stamina")
    FOnStaminaChanged OnStaminaChanged;

    UPROPERTY(BlueprintAssignable, Category = "Stamina")
    FOnStaminaStateChanged OnStaminaStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Stamina")
    FOnExhausted OnExhausted;

    UPROPERTY(BlueprintAssignable, Category = "Stamina")
    FOnRecovered OnRecovered;

private:
    FTimerHandle ExhaustionRecoveryTimerHandle;
    float LastStaminaChangeTime;

    UFUNCTION(Server, Reliable)
    void Server_ConsumeStamina(float Amount);

    UFUNCTION(Server, Reliable)
    void Server_SetSprintState(bool bSprinting);

    UFUNCTION(Server, Reliable)
    void Server_SetRunState(bool bRunning);

    void EndExhaustionRecoveryDelay();
};