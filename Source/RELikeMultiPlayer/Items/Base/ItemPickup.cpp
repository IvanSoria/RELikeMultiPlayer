// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemPickup.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "../../Player/Character/RELikeMultiPlayerCharacter.h"
#include "../../Components/Inventory/InventoryComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

AItemPickup::AItemPickup()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    // Create collision sphere
    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    RootComponent = CollisionSphere;
    CollisionSphere->SetSphereRadius(50.0f);
    CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create mesh component
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -50.0f));
    
    // Add floating effect
    MeshComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
}

void AItemPickup::BeginPlay()
{
    Super::BeginPlay();

    // Only bind overlap events on server
    if (HasAuthority())
    {
        CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AItemPickup::OnSphereBeginOverlap);
    }

    // Add a simple floating animation in Blueprint or here
    // This is a simple rotation, you can enhance it in Blueprint
    if (MeshComponent)
    {
        FRotator CurrentRotation = MeshComponent->GetRelativeRotation();
        CurrentRotation.Yaw += 1.0f;
        MeshComponent->SetRelativeRotation(CurrentRotation);
    }
}

void AItemPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AItemPickup, bIsActive);
}

void AItemPickup::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // Only process on server
    if (!HasAuthority()) return;

    // Check if pickup is active and auto-pickup is enabled
    if (bIsActive && bAutoPickup)
    {
        PickupItem(OtherActor);
    }
}

void AItemPickup::PickupItem(AActor* Picker)
{
    // If called on client, forward to server
    if (GetLocalRole() < ROLE_Authority)
    {
        Server_PickupItem(Picker);
        return;
    }

    // Validate picker and active state
    if (!Picker || !bIsActive) return;

    // Try to cast to our character class
    ARELikeMultiPlayerCharacter* Character = Cast<ARELikeMultiPlayerCharacter>(Picker);
    if (!Character) return;

    // Get inventory component
    UInventoryComponent* Inventory = Character->GetInventoryComponent();
    if (!Inventory) return;

    // Try to add item to inventory
    if (Inventory->AddItem(ItemID, Quantity))
    {
        // Successfully picked up
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-8, 2.0f, FColor::Green, 
                FString::Printf(TEXT("Picked up %d x %s"), Quantity, *ItemID));
        }

        // Deactivate pickup
        DeactivatePickup();

        // Handle respawn if enabled
        if (RespawnTime > 0.0f)
        {
            GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AItemPickup::RespawnPickup, RespawnTime, false);
        }
        else
        {
            // Destroy after a short delay if no respawn
            SetLifeSpan(0.5f);
        }
    }
    else
    {
        // Inventory full
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-9, 2.0f, FColor::Red, TEXT("Inventory Full!"));
        }
    }
}

void AItemPickup::SetItemData(const FString& InItemID, int32 InQuantity)
{
    ItemID = InItemID;
    Quantity = FMath::Max(1, InQuantity);
}

void AItemPickup::DeactivatePickup()
{
    bIsActive = false;
    OnRep_IsActive();
}

void AItemPickup::RespawnPickup()
{
    if (HasAuthority())
    {
        bIsActive = true;
        OnRep_IsActive();
    }
}

void AItemPickup::OnRep_IsActive()
{
    // Update visibility and collision based on active state
    SetActorHiddenInGame(!bIsActive);
    SetActorEnableCollision(bIsActive);

    // Optional: Add effects for pickup/respawn
    if (bIsActive)
    {
        // Spawn effect when item appears
        // You can add particle effects or sounds here
    }
    else
    {
        // Pickup effect when item is collected
        // You can add particle effects or sounds here
    }
}

void AItemPickup::Server_PickupItem_Implementation(AActor* Picker)
{
    PickupItem(Picker);
}
