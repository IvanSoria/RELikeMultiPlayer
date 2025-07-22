// Fill out your copyright notice in the Description page of Project Settings.


#include "MainAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"

void UMainAnimInstance::NativeInitializeAnimation()
{
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner();
	}

	if (Character == nullptr)
	{
		Character = Cast<ACharacter>(Pawn);
	}
}

void UMainAnimInstance::UpdateAnimationProperties()
{
    if (Pawn == nullptr)
    {
        Pawn = TryGetPawnOwner();
    }

    if (Character == nullptr)
    {
        Character = Cast<ACharacter>(Pawn);
    }

    if (Pawn && Character)
    {
        FVector Speed = Pawn->GetVelocity();
        FVector LateralSpeed = FVector(Speed.X, Speed.Y, 0.f);
        MovementSpeed = LateralSpeed.Size();

        bIsInAir = Pawn->GetMovementComponent()->IsFalling();
        bIsCrouched = Character->bIsCrouched;
        
        // Debug output
        if (GEngine)
        {
            FString DebugMsg = FString::Printf(
                TEXT("Crouch: %s, CapsuleHalfHeight: %.2f, Location Z: %.2f"),
                bIsCrouched ? TEXT("Yes") : TEXT("No"),
                Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),
                Character->GetActorLocation().Z
            );
            GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Yellow, DebugMsg);
        }
    }
}