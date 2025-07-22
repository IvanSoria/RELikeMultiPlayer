// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/DataTable.h"
#include "InventoryComponent.generated.h"

// Item Categories
UENUM(BlueprintType)
enum class EItemCategory : uint8
{
    None         UMETA(DisplayName = "None"),
    Medical      UMETA(DisplayName = "Medical"),
    Tools        UMETA(DisplayName = "Tools"),
    Weapons      UMETA(DisplayName = "Weapons"),
    Resources    UMETA(DisplayName = "Resources"),
    Lore         UMETA(DisplayName = "Lore")
};

// Item Data Structure
USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FString ItemID;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FString ItemName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FString Description;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    EItemCategory Category;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    UTexture2D* Icon;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    bool bStackable = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 MaxStackSize = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSubclassOf<class AItemPickup> PickupClass;

    FItemData()
    {
        ItemID = "";
        ItemName = "Unknown Item";
        Description = "";
        Category = EItemCategory::None;
        Icon = nullptr;
        bStackable = false;
        MaxStackSize = 1;
    }
};

// Inventory Slot
USTRUCT(BlueprintType)
struct FInventorySlot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString ItemID;

    UPROPERTY(BlueprintReadOnly)
    int32 Quantity;

    UPROPERTY(BlueprintReadOnly)
    int32 SlotIndex;

    FInventorySlot()
    {
        ItemID = "";
        Quantity = 0;
        SlotIndex = -1;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryUpdated, int32, SlotIndex, const FInventorySlot&, Slot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemPickedUp, const FString&, ItemID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemDropped, const FString&, ItemID, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemUsed, const FString&, ItemID, int32, SlotIndex, int32, Quantity);

// Forward declarations
class UHealthComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RELIKEMULTIPLAYER_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

	protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Inventory Properties
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
    int32 InventoryColumns = 4;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
    int32 InventoryRows = 2;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
    UDataTable* ItemDataTable;

    UPROPERTY(ReplicatedUsing = OnRep_Inventory, BlueprintReadOnly, Category = "Inventory")
    TArray<FInventorySlot> Inventory;

    // Replication
    UFUNCTION()
    void OnRep_Inventory();

    // Internal Functions
    int32 GetTotalSlots() const { return InventoryColumns * InventoryRows; }
    bool IsValidSlotIndex(int32 SlotIndex) const;
    FItemData* GetItemData(const FString& ItemID) const;
    int32 FindFirstEmptySlot() const;
    int32 FindItemSlot(const FString& ItemID) const;
    int32 FindPartialStackSlot(const FString& ItemID) const;

public:
    // Public Functions
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddItem(const FString& ItemID, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItem(int32 SlotIndex, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool DropItem(int32 SlotIndex, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool UseItem(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool SwapItems(int32 FromSlot, int32 ToSlot);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TransferItem(int32 SlotIndex, UInventoryComponent* TargetInventory, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    FInventorySlot GetSlot(int32 SlotIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool HasItem(const FString& ItemID, int32 RequiredQuantity = 1) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 GetItemCount(const FString& ItemID) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    TArray<FInventorySlot> GetAllItems() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ClearInventory();

    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryUpdated OnInventoryUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnItemPickedUp OnItemPickedUp;

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnItemDropped OnItemDropped;

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnItemUsed OnItemUsed;

private:
    // Server RPCs
    UFUNCTION(Server, Reliable)
    void Server_AddItem(const FString& ItemID, int32 Quantity);

    UFUNCTION(Server, Reliable)
    void Server_RemoveItem(int32 SlotIndex, int32 Quantity);

    UFUNCTION(Server, Reliable)
    void Server_DropItem(int32 SlotIndex, int32 Quantity);

    UFUNCTION(Server, Reliable)
    void Server_UseItem(int32 SlotIndex);

    UFUNCTION(Server, Reliable)
    void Server_SwapItems(int32 FromSlot, int32 ToSlot);

    UFUNCTION(Server, Reliable)
    void Server_TransferItem(int32 SlotIndex, UInventoryComponent* TargetInventory, int32 Quantity);

    // Multicast RPCs
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_OnItemPickedUp(const FString& ItemID);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_OnItemDropped(const FString& ItemID, int32 Quantity);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_OnItemUsed(const FString& ItemID, int32 SlotIndex, int32 Quantity);
};
