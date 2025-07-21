// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MenuInterface.generated.h"

class UUserWidget;	// Forward declaration

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UMenuInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class STEAMMULTIPLAYERSESSIONS_API IMenuInterface
{
	GENERATED_BODY()

		// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void Host(const FString ServerName = TEXT("Ivan's Test Session")) = 0;
	//virtual void Join(const FString& SessionName) = 0;
	virtual void Join(uint32 index) = 0;
	virtual void LoadMenuWidget(UUserWidget* MenuWidget) = 0;
	virtual void RefreshServerList() = 0;
};
