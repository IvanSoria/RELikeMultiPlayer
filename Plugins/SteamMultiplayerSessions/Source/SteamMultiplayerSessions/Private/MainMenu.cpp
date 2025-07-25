// Fill out your copyright notice in the Description page of Project Settings.

#include "MainMenu.h"

#include "UObject/ConstructorHelpers.h"

#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "MultiplayerSessionsSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

#include "ServerRow.h"

UMainMenu::UMainMenu(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
	ConstructorHelpers::FClassFinder<UUserWidget> ServerRowBPClass(TEXT("/SteamMultiplayerSessions/Widgets/WIG_ServerRow"));
	if (!ensure(ServerRowBPClass.Class != nullptr))
		return;

	ServerRowClass = ServerRowBPClass.Class;
}

bool UMainMenu::Initialize()
{
	bool Success = Super::Initialize();

	UGameInstance *GameInstance = GetGameInstance();

	if (!ensure(GameInstance != nullptr))
		return false;

	MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();

	if (!ensure(MultiplayerSessionsSubsystem != nullptr))
		return false;

	MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
	MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySessionComplete);
	MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSessionComplete);
	MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddDynamic(this, &ThisClass::OnJoinSessionComplete);

	if (!Success)
		return false;

	if (!ensure(HostButton != nullptr))
		return false;
	HostButton->OnClicked.AddDynamic(this, &UMainMenu::OpenHostMenu);

	if (!ensure(CancelHostMenuButton != nullptr))
		return false;
	CancelHostMenuButton->OnClicked.AddDynamic(this, &UMainMenu::OpenMainMenu);

	if (!ensure(ConfirmHostMenuButton != nullptr))
		return false;
	ConfirmHostMenuButton->OnClicked.AddDynamic(this, &UMainMenu::HostServer);

	if (!ensure(JoinButton != nullptr))
		return false;
	JoinButton->OnClicked.AddDynamic(this, &UMainMenu::OpenJoinMenu);

	if (!ensure(QuitButton != nullptr))
		return false;
	QuitButton->OnClicked.AddDynamic(this, &UMainMenu::QuitPressed);

	if (!ensure(CancelJoinMenuButton != nullptr))
		return false;
	CancelJoinMenuButton->OnClicked.AddDynamic(this, &UMainMenu::OpenMainMenu);

	if (!ensure(ConfirmJoinMenuButton != nullptr))
		return false;
	ConfirmJoinMenuButton->OnClicked.AddDynamic(this, &UMainMenu::JoinServer);

	return true;
}

void UMainMenu::OnCreateSession(bool bWasSuccessful)
{

	// check if GEngine is valid
	if (!ensure(GEngine != nullptr))
		return;

	if (bWasSuccessful)
	{

		// add message to screen Session created successfully!
		GEngine->AddOnScreenDebugMessage(-1,
										 5.f,
										 FColor::Red, TEXT("Session created successfully!"));

		// declare pointer to UWorld and assign it to GetWorld()
		TObjectPtr<UWorld> World = GetWorld();

		// check if World is valid
		if (!ensure(World != nullptr))
			return;

		// server tavel to the lobby map
		World->ServerTravel(TEXT("/Game/ThirdPerson/Maps/Lobby?listen"));
	}
	else
	{
		// add message to screen Session created failed!
		GEngine->AddOnScreenDebugMessage(-1,
										 5.f,
										 FColor::Red, TEXT("Session created failed!"));
	}
}

void UMainMenu::OnJoinSessionComplete(FJoinSessionResultWrapper JoinSessionResult)
{
	if (!ensure(GEngine != nullptr))
		return;

	EOnJoinSessionCompleteResult::Type Result = UMultiplayerSessionsSubsystem::ConvertEOnJoinSessionCompleteResultWrapperToType(JoinSessionResult.Result);

	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		FString ErrorMessage = TEXT("Session join failed!");
		switch (Result)
		{
		case EOnJoinSessionCompleteResult::SessionIsFull:
			ErrorMessage = TEXT("Session is full!");
			break;
		case EOnJoinSessionCompleteResult::SessionDoesNotExist:
			ErrorMessage = TEXT("Session does not exist!");
			break;
		case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
			ErrorMessage = TEXT("Could not retrieve address!");
			break;
		case EOnJoinSessionCompleteResult::AlreadyInSession:
			ErrorMessage = TEXT("Already in session!");
			break;
		default:
			ErrorMessage = TEXT("Unknown error!");
			break;
		}
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, ErrorMessage);
		return;
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Session join successful!"));

	UWorld *World = GetWorld();
	if (!ensure(World != nullptr))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("World is null!"));
		return;
	}

	APlayerController *PlayerController = World->GetFirstPlayerController();
	if (!ensure(PlayerController != nullptr))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("PlayerController is null!"));
		return;
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Joining %s"), *JoinSessionResult.Address));
	// should travel to the address of the session -- is currently loading a black screen then loading the main menu again
	PlayerController->ClientTravel(JoinSessionResult.Address, ETravelType::TRAVEL_Absolute);
}

void UMainMenu::OnDestroySessionComplete(bool bWasSuccessful)
{
}

void UMainMenu::OnStartSessionComplete(bool bWasSuccessful)
{
}

void UMainMenu::OpenHostMenu()
{
	if (!ensure(MenuSwitcher != nullptr))
		return;
	if (!ensure(HostMenu != nullptr))
		return;
	MenuSwitcher->SetActiveWidget(HostMenu);
}

void UMainMenu::HostServer()
{
	if (MenuInterface != nullptr)
	{
		FString ServerName = ServerHostName->GetText().ToString();
		MenuInterface->Host(ServerName);
	}
}

void UMainMenu::SetServerList(TArray<FServerData> ServerNames)
{
	UWorld *World = this->GetWorld();
	if (!ensure(World != nullptr))
		return;

	ServerList->ClearChildren();

	uint32 i = 0;
	for (const FServerData &ServerData : ServerNames)
	{
		UServerRow *Row = CreateWidget<UServerRow>(World, ServerRowClass);
		if (!ensure(Row != nullptr))
			return;

		Row->ServerName->SetText(FText::FromString(ServerData.Name));
		Row->HostUser->SetText(FText::FromString(ServerData.HostUserName));
		FString FractionText = FString::Printf(TEXT("%d/%d"), ServerData.CurrentPlayers, ServerData.MaxPlayers);
		Row->ConnectionFraction->SetText(FText::FromString(FractionText));
		Row->Setup(this, i++);

		ServerList->AddChild(Row);
	}
}

void UMainMenu::SelectIndex(uint32 Index)
{
	SelectedIndex = Index;
	UpdateChildren();
}

void UMainMenu::UpdateChildren()
{
	for (int32 i = 0; i < ServerList->GetChildrenCount(); ++i)
	{
		auto Row = Cast<UServerRow>(ServerList->GetChildAt(i));
		if (Row != nullptr)
		{
			Row->Selected = (SelectedIndex.IsSet() && SelectedIndex.GetValue() == i);
		}
	}
}

void UMainMenu::JoinServer()
{
	if (SelectedIndex.IsSet() && MenuInterface != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Selected index %d."), SelectedIndex.GetValue());
		MenuInterface->Join(SelectedIndex.GetValue());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Selected index not set."));
	}
}

void UMainMenu::OpenJoinMenu()
{
	if (!ensure(MenuSwitcher != nullptr))
		return;
	if (!ensure(JoinMenu != nullptr))
		return;
	MenuSwitcher->SetActiveWidget(JoinMenu);
	if (MenuInterface != nullptr)
	{
		MenuInterface->RefreshServerList();
	}
}

void UMainMenu::OpenMainMenu()
{
	if (!ensure(MenuSwitcher != nullptr))
		return;
	if (!ensure(JoinMenu != nullptr))
		return;
	MenuSwitcher->SetActiveWidget(MainMenu);
}

void UMainMenu::QuitPressed()
{
	UWorld *World = GetWorld();
	if (!ensure(World != nullptr))
		return;

	APlayerController *PlayerController = World->GetFirstPlayerController();
	if (!ensure(PlayerController != nullptr))
		return;

	PlayerController->ConsoleCommand("quit");
}
