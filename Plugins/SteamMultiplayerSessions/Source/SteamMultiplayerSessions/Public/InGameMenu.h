// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MenuWidget.h"
#include "InGameMenu.generated.h"

class UButton;
class UMainMenu;

/**
 * 
 */
UCLASS()
class STEAMMULTIPLAYERSESSIONS_API UInGameMenu : public UMenuWidget
{
	GENERATED_BODY()
	
public:

	void InGameMenu(const FObjectInitializer& ObjectInitializer);

protected:
	virtual bool Initialize();

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CancelButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> QuitButton;

	UFUNCTION()
	void CancelPressed();

	UFUNCTION()
	void QuitPressed();

	TSubclassOf<UUserWidget> MainMenuClass;

	TObjectPtr<UMainMenu> MainMenu;
};
