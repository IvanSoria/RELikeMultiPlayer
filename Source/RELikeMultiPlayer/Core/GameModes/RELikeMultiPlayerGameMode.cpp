// Copyright Epic Games, Inc. All Rights Reserved.

#include "RELikeMultiPlayerGameMode.h"
#include "../../Player/Character/RELikeMultiPlayerCharacter.h"
#include "UObject/ConstructorHelpers.h"

ARELikeMultiPlayerGameMode::ARELikeMultiPlayerGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// No custom HUD class needed - PlayerHUDWidget will be created by PlayerController or Character
}

void ARELikeMultiPlayerGameMode::BeginPlay()
{
	Super::BeginPlay();
}