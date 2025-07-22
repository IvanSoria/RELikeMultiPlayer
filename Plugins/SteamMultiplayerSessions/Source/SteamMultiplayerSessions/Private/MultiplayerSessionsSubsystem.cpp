// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "MainMenu.h"
#include "InGameMenu.h"
#include "HAL/IConsoleManager.h"
#include "Online/OnlineSessionNames.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem() : CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
																 FindSessionCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
																 JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
																 DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
																 StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))
{

	ConstructorHelpers::FClassFinder<UUserWidget> MenuBPClass(TEXT("/SteamMultiplayerSessions/Widgets/WIG_MainMenu"));

	if (!ensure(MenuBPClass.Class != nullptr))
		return;

	MenuClass.Add(MenuBPClass.Class);

	ConstructorHelpers::FClassFinder<UUserWidget> InGameMenuBPClass(TEXT("/SteamMultiplayerSessions/Widgets/WIG_InGameMenu"));

	if (!ensure(InGameMenuBPClass.Class != nullptr))
		return;

	MenuClass.Add(InGameMenuBPClass.Class);

	// get the online subsystem and store it in the session interface pointer
	IOnlineSubsystem *OnlineSubsystem = IOnlineSubsystem::Get();

	// check if the online subsystem is valid and return if not
	if (!ensure(OnlineSubsystem != nullptr))
	{
		// add debug message to screen "OnlineSubsystem is not valid" in new key
		GEngine->AddOnScreenDebugMessage(
			1,
			15.f,
			FColor::Red,
			FString::Printf(TEXT("OnlineSubsystem is not valid")));
		return;
	}

	// set session interface pointer to the online subsystem session interface
	SessionInterface = OnlineSubsystem->GetSessionInterface();

	/**Work in progress*/
	/*if(SessionInterface.IsValid())
	{
		ServerListRefreshedCompleteDelegate.AddDynamic(this, &ThisClass::OnServerListRefreshedComplete);
	}*/
}

void UMultiplayerSessionsSubsystem::LoadMenuWidget(UUserWidget *MenuWidget)
{
	if (!ensure(!MenuClass.IsEmpty()))
		return;

	WidgetToLoad = MenuWidget;

	if (!ensure(WidgetToLoad != nullptr))
		return;

	if (WidgetToLoad->IsA(UMainMenu::StaticClass()))
	{

		Cast<UMainMenu>(WidgetToLoad)->Setup();
		Cast<UMainMenu>(WidgetToLoad)->SetMenuInterface(this);
	}
	else if (WidgetToLoad->IsA(UInGameMenu::StaticClass()))
	{

		Cast<UInGameMenu>(WidgetToLoad)->Setup();
		Cast<UInGameMenu>(WidgetToLoad)->SetMenuInterface(this);
	}
}

void UMultiplayerSessionsSubsystem::Host(const FString ServerName)
{

	DesiredServerName = ServerName;
	if (SessionInterface.IsValid())
	{
		FNamedOnlineSession *ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
		if (ExistingSession != nullptr)
		{
			SessionInterface->DestroySession(NAME_GameSession);
		}
		else
		{
			CreateSession();
		}
	}
}

void UMultiplayerSessionsSubsystem::Join(uint32 Index)
{
	// ensure that the session interface is valid
	if (!ensure(SessionInterface.IsValid()))
		return;
	// ensure that the LastSessionSearch is valid
	if (!ensure(LastSessionSearch.IsValid()))
		return;

	if (WidgetToLoad != nullptr)
	{
		if (WidgetToLoad->IsA(UMainMenu::StaticClass()))
		{
			// cast WidgetToLoad to UMainMenu
			Cast<UMainMenu>(WidgetToLoad)->Teardown();
		}
		else
		{
			// add debug message to screen "WidgetToLoad is not a valid type" in new key
			GEngine->AddOnScreenDebugMessage(
				9,
				15.f,
				FColor::Red,
				FString::Printf(TEXT("WidgetToLoad is not a valid type")));
		}
	}

	// Store the search result in a wrapper
	OnlineSessionSearchResultWrapper.SearchResult = LastSessionSearch->SearchResults[Index];

	JoinSession();

	return;
}

/*void UMultiplayerSessionsSubsystem::Join(const FString& SessionName)
{
	if (!SessionInterface.IsValid()) return;
	if (!LastSessionSearch.IsValid()) return;

	//check to see if SessionName is valid/exists in the search results
	for (const FOnlineSessionSearchResult& SearchResult : LastSessionSearch->SearchResults)
	{
		if (SearchResult.GetSessionIdStr() == SessionName)
		{
			if (WidgetToLoad != nullptr)
			{
				//WidgetToLoad->Teardown();

				//cast WidgetToLoad to UMainMenu
				Cast<UMainMenu>(WidgetToLoad)->Teardown();
			}
			else
			{
				//add debug message to screen "WidgetToLoad is not a valid type" in new key
				GEngine->AddOnScreenDebugMessage(
					9,
					15.f,
					FColor::Red,
					FString::Printf(TEXT("WidgetToLoad is not a valid type"))
				);
			}

			FOnlineSessionSearchResultWrapper SearchResultWrapper;
			SearchResultWrapper.SearchResult = SearchResult;

			JoinSession(SearchResultWrapper);
			return;
		}
		else
		{
			//add debug message to screen "Session not found" and display the name of trhe session
			GEngine->AddOnScreenDebugMessage(
				10,
				15.f,
				FColor::Red,
				FString::Printf(TEXT("Session not found: %s"), *SessionName)
			);
		}
	}

}*/

/*void UMultiplayerSessionsSubsystem::JoinWrapper(const TArray<FString>& Args)
{
	FString ServerName = "";
	//check if the number of arguments is greater than 0
	if (Args.Num() > 0)
	{
		for ( FString Arg : Args)
		{
			ServerName += Arg + " ";
		}
		//trim the whitespace from the end of the string
		ServerName.TrimEndInline();

		//Refresh the server list and wait for the search to complete
		RefreshServerList();


		//call the Join function with the ServerName
		Join(ServerName);

	}
	else
	{
		//add debug message to screen "Please enter a server name" in new key
		GEngine->AddOnScreenDebugMessage(
			10,
			15.f,
			FColor::Red,
			FString::Printf(TEXT("Please enter a server name"))
		);
	}
}*/

void UMultiplayerSessionsSubsystem::RefreshServerList()
{
	LastSessionSearch = MakeShareable<FOnlineSessionSearch>(new FOnlineSessionSearch());
	if (LastSessionSearch.IsValid())
	{
		// SessionSearch->bIsLanQuery = true;
		LastSessionSearch->MaxSearchResults = 10000;
		LastSessionSearch->QuerySettings.Set(SEARCH_KEYWORDS, true, EOnlineComparisonOp::Equals);
		UE_LOG(LogTemp, Warning, TEXT("Starting Find Session"));
		FindSessions();
		// SessionInterface->FindSessions(0, LastSessionSearch.ToSharedRef());
		// OnServerListRefreshedComplete.Broadcast();
	}
}

void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections)
{
	// check to see if GEngine is valid
	if (!ensure(GEngine != nullptr))
		return;
	// Display the number of public connections and the match type
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("CreateSession: NumPublicConnections: %d, ServerName: %s"), NumPublicConnections, *DesiredServerName));

	// check if the session interface is valid
	if (!ensure(SessionInterface.IsValid()))
		return;

	// get existing session name
	auto ExistingSessionName = SessionInterface->GetNamedSession(NAME_GameSession);

	// check if existing session is not null
	if (ExistingSessionName != nullptr)
	{
		// add debug message to screen "ExistingSessionName is not null"
		GEngine->AddOnScreenDebugMessage(
			4,
			15.f,
			FColor::Blue,
			FString::Printf(TEXT("ExistingSessionName is not null")));

		// destroy any existing sessions
		SessionInterface->DestroySession(NAME_GameSession);

		// add debug message to screen "Session Destroyed" in new key
		GEngine->AddOnScreenDebugMessage(
			5,
			15.f,
			FColor::Blue,
			FString::Printf(TEXT("Session Destroyed")));
	}

	// bind delegate for session creation, stored in CreateSessionCompleteDelegateHandle so we can later remove it
	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	// delcare and assign Shared Pointer to new session settings
	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());

	LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	LastSessionSettings->NumPublicConnections = NumPublicConnections;
	LastSessionSettings->NumPrivateConnections = 0;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->bUsesPresence = true;
	LastSessionSettings->bIsDedicated = false;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->bUseLobbiesIfAvailable = true;
	LastSessionSettings->Set(SERVER_NAME_SETTINGS_KEY, DesiredServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	const TObjectPtr<ULocalPlayer> LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();

	// bind delegate for session creation,
	if (!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings))
	{
		// if session creation fails, remove the delegate
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);

		// add debug message to screen "Session Creation Failed" in new key
		GEngine->AddOnScreenDebugMessage(
			6,
			15.f,
			FColor::Red,
			FString::Printf(TEXT("Session Creation Failed")));

		// broadcast our own custom delegate
		MultiplayerOnCreateSessionComplete.Broadcast(false);
	}
	else
	{
		// add debug message to screen "Session Creation Succeeded" in new key
		GEngine->AddOnScreenDebugMessage(
			6,
			15.f,
			FColor::Blue,
			FString::Printf(TEXT("Session Creation Succeeded")));
	}
}

void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
	// check if the session interface is valid
	if (!ensure(SessionInterface.IsValid()))
		return;

	// bind delegate for finding session
	FindSessionCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionCompleteDelegate);

	const TObjectPtr<ULocalPlayer> LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();

	// check if LocalPlayer is valid
	if (!ensure(LocalPlayer != nullptr))
		return;

	// assign Shared Pointer to new session settings
	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());

	LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	LastSessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);

	// start the session search
	if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()))
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionCompleteDelegateHandle);
		MultiplayerOnFindSessionsComplete.Broadcast(FOnlineSessionSearchResultWrapper(), false);
		return;
	}
}

void UMultiplayerSessionsSubsystem::JoinSession()
{
	// check if the session interface is valid
	if (!ensure(SessionInterface.IsValid()))
	{
		FailedJoinSessionCleanUp();
		return;
	}

	OnlineSessionSearchResultWrapper.SearchResult.Session.SessionSettings.bUsesPresence = OnlineSessionSearchResultWrapper.SearchResult.Session.SessionSettings.bUseLobbiesIfAvailable = true;

	// bind delegate for joining session
	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	// get world and store it in a variable
	TObjectPtr<UWorld> World = GetWorld();

	// check if world is valid
	if (!ensure(World != nullptr))
	{
		FailedJoinSessionCleanUp();
		return;
	}

	// get the Local Player Controller and store it in a variable
	const TObjectPtr<APlayerController> LocalPlayer = World->GetGameInstance()->GetFirstLocalPlayerController();

	// check if player controller is valid
	if (!ensure(LocalPlayer != nullptr))
	{
		FailedJoinSessionCleanUp();
		return;
	}

	// join the session found in the search results with the session id and the player controller then return
	if (!SessionInterface->JoinSession(0, NAME_GameSession, OnlineSessionSearchResultWrapper.SearchResult))
	{
		FailedJoinSessionCleanUp();
		return;
	}
}

void UMultiplayerSessionsSubsystem::FailedJoinSessionCleanUp()
{
	EOnJoinSessionCompleteResultWrapper ResultWrapper = EOnJoinSessionCompleteResultWrapper::UnknownError;
	JoinSessionResultWrapper.Result = ResultWrapper;
	MultiplayerOnJoinSessionComplete.Broadcast(JoinSessionResultWrapper);
}

void UMultiplayerSessionsSubsystem::DestroySession()
{
}

void UMultiplayerSessionsSubsystem::StartSession()
{
}

EOnJoinSessionCompleteResultWrapper UMultiplayerSessionsSubsystem::ConvertEOnJoinSessionCompleteResultToWrapper(EOnJoinSessionCompleteResult::Type Result)
{
	switch (Result)
	{
	case EOnJoinSessionCompleteResult::Success:
		return EOnJoinSessionCompleteResultWrapper::Success;
	case EOnJoinSessionCompleteResult::AlreadyInSession:
		return EOnJoinSessionCompleteResultWrapper::AlreadyInSession;
	case EOnJoinSessionCompleteResult::SessionIsFull:
		return EOnJoinSessionCompleteResultWrapper::SessionIsFull;
	case EOnJoinSessionCompleteResult::SessionDoesNotExist:
		return EOnJoinSessionCompleteResultWrapper::SessionDoesNotExist;
	case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
		return EOnJoinSessionCompleteResultWrapper::CouldNotRetrieveAddress;
	default:
		return EOnJoinSessionCompleteResultWrapper::UnknownError;
	}
}

EOnJoinSessionCompleteResult::Type UMultiplayerSessionsSubsystem::ConvertEOnJoinSessionCompleteResultWrapperToType(EOnJoinSessionCompleteResultWrapper ResultWrapper)
{
	switch (ResultWrapper)
	{
	case EOnJoinSessionCompleteResultWrapper::Success:
		return EOnJoinSessionCompleteResult::Success;
	case EOnJoinSessionCompleteResultWrapper::AlreadyInSession:
		return EOnJoinSessionCompleteResult::AlreadyInSession;
	case EOnJoinSessionCompleteResultWrapper::SessionIsFull:
		return EOnJoinSessionCompleteResult::SessionIsFull;
	case EOnJoinSessionCompleteResultWrapper::SessionDoesNotExist:
		return EOnJoinSessionCompleteResult::SessionDoesNotExist;
	case EOnJoinSessionCompleteResultWrapper::CouldNotRetrieveAddress:
		return EOnJoinSessionCompleteResult::CouldNotRetrieveAddress;
	default:
		return EOnJoinSessionCompleteResult::UnknownError;
	}
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	// check if WidgetToLoad is valid
	if (WidgetToLoad != nullptr)
	{
		if (WidgetToLoad->IsA(UMainMenu::StaticClass()))
		{
			// cast WidgetToLoad to UMainMenu
			Cast<UMainMenu>(WidgetToLoad)->Teardown();
		}
		else if (WidgetToLoad->IsA(UInGameMenu::StaticClass()))
		{
			// cast WidgetToLoad to UInGameMenu
			Cast<UInGameMenu>(WidgetToLoad)->Teardown();
		}
		else
		{
			// add debug message to screen "WidgetToLoad is not a valid type" in new key
			GEngine->AddOnScreenDebugMessage(
				9,
				15.f,
				FColor::Red,
				FString::Printf(TEXT("WidgetToLoad is not a valid type")));
		}
	}

	// check if the session interface is valid
	if (!ensure(SessionInterface.IsValid()))
		return;

	// remove the delegate
	SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);

	// broadcast our own custom delegate
	MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);

	if (!ensure(GEngine != nullptr))
		return;

	if (bWasSuccessful)
		GEngine->AddOnScreenDebugMessage(
			7,
			15.f,
			FColor::Blue,
			FString::Printf(TEXT("Session Created %s"), *SessionName.ToString()));
	else
		GEngine->AddOnScreenDebugMessage(
			7,
			15.f,
			FColor::Red,
			FString::Printf(TEXT("Session Creation Failed %s"), *SessionName.ToString()));
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	// check if the session interface is valid
	if (!ensure(SessionInterface.IsValid()))
		return;

	SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionCompleteDelegateHandle);

	TArray<FServerData> ServerNames;
	for (const FOnlineSessionSearchResult &SearchResult : LastSessionSearch->SearchResults)
	{
		UE_LOG(LogTemp, Warning, TEXT("Found session names: %s"), *SearchResult.GetSessionIdStr());
		FServerData Data;
		Data.MaxPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;
		Data.CurrentPlayers = Data.MaxPlayers - SearchResult.Session.NumOpenPublicConnections;
		Data.HostUserName = SearchResult.Session.OwningUserName;
		FString ServerName;
		if (SearchResult.Session.SessionSettings.Get(SERVER_NAME_SETTINGS_KEY, ServerName))
		{
			Data.Name = ServerName;
		}
		else
		{
			Data.Name = "Could not find name.";
		}
		ServerNames.Add(Data);
	}

	if (WidgetToLoad->IsA(UMainMenu::StaticClass()))
	{
		// cast WidgetToLoad to UMainMenu
		Cast<UMainMenu>(WidgetToLoad)->SetServerList(ServerNames);
	}
	else
	{
		// add debug message to screen "WidgetToLoad is not a valid type" in new key
		GEngine->AddOnScreenDebugMessage(
			9,
			15.f,
			FColor::Red,
			FString::Printf(TEXT("WidgetToLoad is not a valid type")));
	}

	OnlineSessionSearchResultWrapper.SearchResults = LastSessionSearch->SearchResults;

	// check if number of search results is greater than 0
	if (OnlineSessionSearchResultWrapper.SearchResults.Num() <= 0)
	{
		// add debug message to screen "Session Not Found" in new key
		GEngine->AddOnScreenDebugMessage(
			8,
			15.f,
			FColor::Red,
			FString::Printf(TEXT("Session Not Found")));

		// broadcast our own custom delegate
		MultiplayerOnFindSessionsComplete.Broadcast(FOnlineSessionSearchResultWrapper(), bWasSuccessful);
		return;
	}

	MultiplayerOnFindSessionsComplete.Broadcast(OnlineSessionSearchResultWrapper, bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	// check if the session interface is valid
	if (!ensure(SessionInterface.IsValid()))
		return;

	// remove the delegate
	SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

	// get the resolved Connect String from the session interface
	FString Address;
	if (!SessionInterface->GetResolvedConnectString(SessionName, Address))
	{
		// add debug message to screen "Could not retrieve Address" in new key
		GEngine->AddOnScreenDebugMessage(
			10,
			15.f,
			FColor::Red,
			FString::Printf(TEXT("Could not retrieve Address")));
	}

	JoinSessionResultWrapper.Address = Address;

	// broadcast our own custom delegate
	JoinSessionResultWrapper.Result = ConvertEOnJoinSessionCompleteResultToWrapper(Result);
	MultiplayerOnJoinSessionComplete.Broadcast(JoinSessionResultWrapper);
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
}

/**Work in progress */
/*void UMultiplayerSessionsSubsystem::OnServerListRefreshedComplete(bool bWasSuccessful)
{
	if(bWasSuccessful)
	{
	//log to the console that the server list has been refreshed
	UE_LOG(LogTemp, Warning, TEXT("Server List Refreshed"));
	}
	else
	{
		//log to the console that the server list has not been refreshed
		UE_LOG(LogTemp, Warning, TEXT("Server List Not Refreshed"));
	}
	MultiplsyerOnServerListRefreshedComplete.Broadcast(bWasSuccessful);
}*/

void UMultiplayerSessionsSubsystem::Initialize(FSubsystemCollectionBase &Collection)
{
	Super::Initialize(Collection);

	// Register the Host console command withe name and a description
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Host"),
		TEXT("Host a game with the specified name"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::HostWrapper)

	);

	// register the Join console command with the name and the description
	/*IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Join"),
		TEXT("Join a game with the specified name"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::JoinWrapper)
	);*/

	/*MultiplsyerOnServerListRefreshedComplete.AddDynamic(this, &ThisClass::)*/
}

void UMultiplayerSessionsSubsystem::Deinitialize()
{
	Super::Deinitialize();
	// Unregister the console command
	IConsoleManager::Get().UnregisterConsoleObject(TEXT("Host"), false);
}

void UMultiplayerSessionsSubsystem::HostWrapper(const TArray<FString> &Args)
{
	FString ServerName = "";
	// check if the number of arguments is greater than 0
	if (Args.Num() > 0)
	{
		for (FString Arg : Args)
		{
			ServerName += Arg + " ";
		}
		// trim the whitespace from the end of the string
		ServerName.TrimEndInline();
		// call the Host function with the ServerName
		Host(ServerName);
	}
	else
	{
		// add debug message to screen "Please enter a server name" in new key
		GEngine->AddOnScreenDebugMessage(
			10,
			15.f,
			FColor::Red,
			FString::Printf(TEXT("Please enter a server name")));
	}
}
