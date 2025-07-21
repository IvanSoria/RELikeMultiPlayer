// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "ItemPickup.generated.h"

UCLASS()
class RELIKEMULTIPLAYER_API AItemPickup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItemPickup();

	protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* CollisionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* MeshComponent;

    // Item Properties
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FString ItemID;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    int32 Quantity = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    bool bAutoPickup = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    float RespawnTime = 0.0f; // 0 means no respawn

    // State
    UPROPERTY(ReplicatedUsing = OnRep_IsActive, BlueprintReadOnly, Category = "State")
    bool bIsActive = true;

    // Overlap Detection
    UFUNCTION()
    void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnRep_IsActive();

    // Internal Functions
    void DeactivatePickup();
    void RespawnPickup();

public:
    // Public Functions
    UFUNCTION(BlueprintCallable, Category = "Item")
    void PickupItem(AActor* Picker);

    UFUNCTION(BlueprintCallable, Category = "Item")
    void SetItemData(const FString& InItemID, int32 InQuantity);

    UFUNCTION(BlueprintCallable, Category = "Item")
    FString GetItemID() const { return ItemID; }

    UFUNCTION(BlueprintCallable, Category = "Item")
    int32 GetQuantity() const { return Quantity; }

    UFUNCTION(BlueprintCallable, Category = "Item")
    bool IsActive() const { return bIsActive; }

private:
    FTimerHandle RespawnTimerHandle;

    // Server RPCs
    UFUNCTION(Server, Reliable)
    void Server_PickupItem(AActor* Picker);
};
