// Copyright Epic Games, Inc. All Rights Reserved.

#include "RELikeMultiPlayerGameMode.h"
#include "../../Player/Character/RELikeMultiPlayerCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

ARELikeMultiPlayerGameMode::ARELikeMultiPlayerGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void ARELikeMultiPlayerGameMode::BeginPlay()
{
	Super::BeginPlay();
}
