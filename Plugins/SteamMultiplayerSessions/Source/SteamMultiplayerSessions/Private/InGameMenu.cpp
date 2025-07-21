// Fill out your copyright notice in the Description page of Project Settings.


#include "InGameMenu.h"
#include "MainMenu.h"
#include "Components/Button.h"


void UInGameMenu::InGameMenu(const FObjectInitializer& ObjectInitializer)
{
	ConstructorHelpers::FClassFinder<UUserWidget> MenuBPClass(TEXT("/SteamMultiplayerSessions/Widgets/WIG_MainMenu"));

	if (!ensure(MenuBPClass.Class != nullptr)) return;

	MainMenuClass = MenuBPClass.Class;

	MainMenu = CreateWidget<UMainMenu>(this, MainMenuClass);
}

bool UInGameMenu::Initialize()
{
	bool Success = Super::Initialize();
	if (!Success) return false;


	if (!ensure(MainMenu != nullptr)) return false;

	if (!ensure(CancelButton != nullptr)) return false;
	CancelButton->OnClicked.AddDynamic(this, &UInGameMenu::CancelPressed);

	if (!ensure(QuitButton != nullptr)) return false;
	QuitButton->OnClicked.AddDynamic(this, &UInGameMenu::QuitPressed);

	return true;
}

void UInGameMenu::CancelPressed()
{
	Teardown();
}


void UInGameMenu::QuitPressed()
{
	if (MenuInterface != nullptr) {
		Teardown();
		MenuInterface->LoadMenuWidget(MainMenu);
	}
}