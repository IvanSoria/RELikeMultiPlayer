// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "RELikeGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class RELIKEMULTIPLAYER_API URELikeGameInstance : public UGameInstance
{
	GENERATED_BODY()


public:

		URELikeGameInstance(const FObjectInitializer &ObjectInitializer);

		virtual void Init();

		/*UFUNCTION(Exec)
		void Host();

		UFUNCTION(Exec)
		void Join(const FString& Address);*/
};
