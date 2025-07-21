// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RELikeMultiPlayerGameMode.generated.h"

UCLASS(minimalapi)
class ARELikeMultiPlayerGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARELikeMultiPlayerGameMode();

private:

	virtual void BeginPlay() override;
	
};



