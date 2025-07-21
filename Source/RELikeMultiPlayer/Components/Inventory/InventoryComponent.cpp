// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryComponent.h"
#include "../../Items/Base/ItemPickup.h"
#include "../Health/HealthComponent.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();

    // Initialize inventory on server
    if (GetOwnerRole() == ROLE_Authority)
    {
        Inventory.SetNum(GetTotalSlots());
        for (int32 i = 0; i < GetTotalSlots(); i++)
        {
            Inventory[i].SlotIndex = i;
        }
    }
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UInventoryComponent, Inventory);
}

void UInventoryComponent::OnRep_Inventory()
{
    // Update UI for all slots
    for (int32 i = 0; i < Inventory.Num(); i++)
    {
        OnInventoryUpdated.Broadcast(i, Inventory[i]);
    }
}

bool UInventoryComponent::IsValidSlotIndex(int32 SlotIndex) const
{
    return SlotIndex >= 0 && SlotIndex < GetTotalSlots();
}

FItemData* UInventoryComponent::GetItemData(const FString& ItemID) const
{
    if (!ItemDataTable) return nullptr;

    static const FString ContextString(TEXT("Item Lookup"));
    return ItemDataTable->FindRow<FItemData>(FName(*ItemID), ContextString);
}

int32 UInventoryComponent::FindFirstEmptySlot() const
{
    for (int32 i = 0; i < Inventory.Num(); i++)
    {
        if (Inventory[i].ItemID.IsEmpty())
        {
            return i;
        }
    }
    return -1;
}

int32 UInventoryComponent::FindItemSlot(const FString& ItemID) const
{
    for (int32 i = 0; i < Inventory.Num(); i++)
    {
        if (Inventory[i].ItemID == ItemID)
        {
            return i;
        }
    }
    return -1;
}

int32 UInventoryComponent::FindPartialStackSlot(const FString& ItemID) const
{
    FItemData* ItemData = GetItemData(ItemID);
    if (!ItemData || !ItemData->bStackable) return -1;

    for (int32 i = 0; i < Inventory.Num(); i++)
    {
        if (Inventory[i].ItemID == ItemID && Inventory[i].Quantity < ItemData->MaxStackSize)
        {
            return i;
        }
    }
    return -1;
}

bool UInventoryComponent::AddItem(const FString& ItemID, int32 Quantity)
{
    if (GetOwnerRole() < ROLE_Authority)
    {
        Server_AddItem(ItemID, Quantity);
        return true; // Assume success on client
    }

    FItemData* ItemData = GetItemData(ItemID);
    if (!ItemData || Quantity <= 0) return false;

    int32 RemainingQuantity = Quantity;

    // If stackable, try to add to existing stacks first
    if (ItemData->bStackable)
    {
        while (RemainingQuantity > 0)
        {
            int32 PartialSlot = FindPartialStackSlot(ItemID);
            if (PartialSlot == -1) break;

            int32 SpaceInStack = ItemData->MaxStackSize - Inventory[PartialSlot].Quantity;
            int32 QuantityToAdd = FMath::Min(RemainingQuantity, SpaceInStack);

            Inventory[PartialSlot].Quantity += QuantityToAdd;
            RemainingQuantity -= QuantityToAdd;

            OnInventoryUpdated.Broadcast(PartialSlot, Inventory[PartialSlot]);
        }
    }

    // Add remaining items to empty slots
    while (RemainingQuantity > 0)
    {
        int32 EmptySlot = FindFirstEmptySlot();
        if (EmptySlot == -1) return false; // Inventory full

        int32 QuantityToAdd = ItemData->bStackable ? 
            FMath::Min(RemainingQuantity, ItemData->MaxStackSize) : 1;

        Inventory[EmptySlot].ItemID = ItemID;
        Inventory[EmptySlot].Quantity = QuantityToAdd;
        RemainingQuantity -= QuantityToAdd;

        OnInventoryUpdated.Broadcast(EmptySlot, Inventory[EmptySlot]);
    }

    Multicast_OnItemPickedUp(ItemID);
    return true;
}

bool UInventoryComponent::RemoveItem(int32 SlotIndex, int32 Quantity)
{
    if (GetOwnerRole() < ROLE_Authority)
    {
        Server_RemoveItem(SlotIndex, Quantity);
        return true;
    }

    if (!IsValidSlotIndex(SlotIndex) || Inventory[SlotIndex].ItemID.IsEmpty()) return false;

    if (Inventory[SlotIndex].Quantity > Quantity)
    {
        Inventory[SlotIndex].Quantity -= Quantity;
    }
    else
    {
        Inventory[SlotIndex].ItemID = "";
        Inventory[SlotIndex].Quantity = 0;
    }

    OnInventoryUpdated.Broadcast(SlotIndex, Inventory[SlotIndex]);
    return true;
}

bool UInventoryComponent::DropItem(int32 SlotIndex, int32 Quantity)
{
    if (GetOwnerRole() < ROLE_Authority)
    {
        Server_DropItem(SlotIndex, Quantity);
        return true;
    }

    if (!IsValidSlotIndex(SlotIndex) || Inventory[SlotIndex].ItemID.IsEmpty()) return false;

    FString ItemID = Inventory[SlotIndex].ItemID;
    int32 QuantityToDrop = FMath::Min(Quantity, Inventory[SlotIndex].Quantity);

    // Spawn pickup in world
    FItemData* ItemData = GetItemData(ItemID);
    if (ItemData && ItemData->PickupClass)
    {
        FVector SpawnLocation = GetOwner()->GetActorLocation() + 
            GetOwner()->GetActorForwardVector() * 100.0f;
        
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        
        AItemPickup* DroppedItem = GetWorld()->SpawnActor<AItemPickup>(
            ItemData->PickupClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
        
        if (DroppedItem)
        {
            DroppedItem->SetItemData(ItemID, QuantityToDrop);
        }
    }

    RemoveItem(SlotIndex, QuantityToDrop);
    Multicast_OnItemDropped(ItemID, QuantityToDrop);
    return true;
}

bool UInventoryComponent::UseItem(int32 SlotIndex)
{
    if (GetOwnerRole() < ROLE_Authority)
    {
        Server_UseItem(SlotIndex);
        return true;
    }

    if (!IsValidSlotIndex(SlotIndex) || Inventory[SlotIndex].ItemID.IsEmpty()) return false;

    FString ItemID = Inventory[SlotIndex].ItemID;
    FItemData* ItemData = GetItemData(ItemID);
    if (!ItemData) return false;

    // Handle item use based on category
    switch (ItemData->Category)
    {
    case EItemCategory::Medical:
        // Heal the player
        if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
        {
            if (UHealthComponent* HealthComp = Character->FindComponentByClass<UHealthComponent>())
            {
                // Different healing amounts based on item
                float HealAmount = 25.0f; // Default for bandages
                if (ItemID == "medkit") HealAmount = 50.0f;
                else if (ItemID == "painpills") HealAmount = 15.0f;
                
                HealthComp->Heal(HealAmount);
                RemoveItem(SlotIndex, 1);
                Multicast_OnItemUsed(ItemID, SlotIndex, 1);
                return true;
            }
        }
        break;
    
    case EItemCategory::Tools:
        // Tools typically don't get consumed when used
        Multicast_OnItemUsed(ItemID, SlotIndex, 0);
        return true;
    
    default:
        break;
    }

    return false;
}

bool UInventoryComponent::SwapItems(int32 FromSlot, int32 ToSlot)
{
    if (GetOwnerRole() < ROLE_Authority)
    {
        Server_SwapItems(FromSlot, ToSlot);
        return true;
    }

    if (!IsValidSlotIndex(FromSlot) || !IsValidSlotIndex(ToSlot)) return false;

    FInventorySlot TempSlot = Inventory[FromSlot];
    Inventory[FromSlot] = Inventory[ToSlot];
    Inventory[ToSlot] = TempSlot;

    // Maintain correct slot indices
    Inventory[FromSlot].SlotIndex = FromSlot;
    Inventory[ToSlot].SlotIndex = ToSlot;

    OnInventoryUpdated.Broadcast(FromSlot, Inventory[FromSlot]);
    OnInventoryUpdated.Broadcast(ToSlot, Inventory[ToSlot]);

    return true;
}

bool UInventoryComponent::TransferItem(int32 SlotIndex, UInventoryComponent* TargetInventory, int32 Quantity)
{
    if (GetOwnerRole() < ROLE_Authority)
    {
        Server_TransferItem(SlotIndex, TargetInventory, Quantity);
        return true;
    }

    if (!IsValidSlotIndex(SlotIndex) || !TargetInventory || Inventory[SlotIndex].ItemID.IsEmpty()) return false;

    FString ItemID = Inventory[SlotIndex].ItemID;
    int32 QuantityToTransfer = FMath::Min(Quantity, Inventory[SlotIndex].Quantity);

    // Try to add item to target inventory
    if (TargetInventory->AddItem(ItemID, QuantityToTransfer))
    {
        // Remove from source inventory
        RemoveItem(SlotIndex, QuantityToTransfer);
        return true;
    }

    return false;
}

// Additional helper functions
FInventorySlot UInventoryComponent::GetSlot(int32 SlotIndex) const
{
    if (IsValidSlotIndex(SlotIndex))
    {
        return Inventory[SlotIndex];
    }
    return FInventorySlot();
}

bool UInventoryComponent::HasItem(const FString& ItemID, int32 RequiredQuantity) const
{
    return GetItemCount(ItemID) >= RequiredQuantity;
}

int32 UInventoryComponent::GetItemCount(const FString& ItemID) const
{
    int32 TotalCount = 0;
    for (const FInventorySlot& Slot : Inventory)
    {
        if (Slot.ItemID == ItemID)
        {
            TotalCount += Slot.Quantity;
        }
    }
    return TotalCount;
}

TArray<FInventorySlot> UInventoryComponent::GetAllItems() const
{
    TArray<FInventorySlot> NonEmptySlots;
    for (const FInventorySlot& Slot : Inventory)
    {
        if (!Slot.ItemID.IsEmpty())
        {
            NonEmptySlots.Add(Slot);
        }
    }
    return NonEmptySlots;
}

void UInventoryComponent::ClearInventory()
{
    if (GetOwnerRole() < ROLE_Authority) return;

    for (int32 i = 0; i < Inventory.Num(); i++)
    {
        Inventory[i].ItemID = "";
        Inventory[i].Quantity = 0;
        OnInventoryUpdated.Broadcast(i, Inventory[i]);
    }
}

// Server RPC Implementations
void UInventoryComponent::Server_AddItem_Implementation(const FString& ItemID, int32 Quantity)
{
    AddItem(ItemID, Quantity);
}

void UInventoryComponent::Server_RemoveItem_Implementation(int32 SlotIndex, int32 Quantity)
{
    RemoveItem(SlotIndex, Quantity);
}

void UInventoryComponent::Server_DropItem_Implementation(int32 SlotIndex, int32 Quantity)
{
    DropItem(SlotIndex, Quantity);
}

void UInventoryComponent::Server_UseItem_Implementation(int32 SlotIndex)
{
    UseItem(SlotIndex);
}

void UInventoryComponent::Server_SwapItems_Implementation(int32 FromSlot, int32 ToSlot)
{
    SwapItems(FromSlot, ToSlot);
}

void UInventoryComponent::Server_TransferItem_Implementation(int32 SlotIndex, UInventoryComponent* TargetInventory, int32 Quantity)
{
    TransferItem(SlotIndex, TargetInventory, Quantity);
}

void UInventoryComponent::Multicast_OnItemPickedUp_Implementation(const FString& ItemID)
{
    OnItemPickedUp.Broadcast(ItemID);
}

void UInventoryComponent::Multicast_OnItemDropped_Implementation(const FString& ItemID, int32 Quantity)
{
    OnItemDropped.Broadcast(ItemID, Quantity);
}

void UInventoryComponent::Multicast_OnItemUsed_Implementation(const FString& ItemID, int32 SlotIndex, int32 Quantity)
{
    OnItemUsed.Broadcast(ItemID, SlotIndex, Quantity);
}
