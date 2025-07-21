// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "MenuInterface.h"
#include "MultiplayerSessionsSubsystem.generated.h"

class UUserWidget;

const static FName SERVER_NAME_SETTINGS_KEY = TEXT("ServerName");

/***
 * This struct is used to hold the results of a session search in a TArray
 * This is necessary because the OnFindSessionsComplete delegate returns a single FOnlineSessionSearchResult
 * but we want to store multiple results in a TArray
 */
USTRUCT(BlueprintType)
struct FOnlineSessionSearchResultWrapper
{
	GENERATED_BODY()

	FOnlineSessionSearchResult SearchResult;
	TArray<FOnlineSessionSearchResult> SearchResults;
};

/**
 * This enum is used to wrap the EOnJoinSessionCompleteResult::Type
 * This is necessary because we can't use the EOnJoinSessionCompleteResult::Type in a USTRUCT
 * We need to use a USTRUCT to store the result of the OnJoinSessionComplete delegate
 * So we need to convert the EOnJoinSessionCompleteResult::Type to our own custom EOnJoinSessionCompleteResultWrapper
 * and vice versa
 * This enum is used to represent the result of joining a session
 * It can be one of the following:
 * Success - The session was joined successfully
 * SessionIsFull - The session is full and cannot be joined
 * SessionDoesNotExist - The session does not exist and cannot be joined
 * CouldNotRetrieveAddress - The address of the session could not be retrieved
 * AlreadyInSession - The player is already in a session and cannot join another
 * UnknownError - An unknown error occurred
 */
UENUM(BlueprintType)
enum class EOnJoinSessionCompleteResultWrapper : uint8
{
	Success,
	SessionIsFull,
	SessionDoesNotExist,
	CouldNotRetrieveAddress,
	AlreadyInSession,
	UnknownError
};

/**
 * This struct is used to store the result of the OnJoinSessionComplete delegate
 * It contains the result of the join session operation and the session name
 * The result is stored as an EOnJoinSessionCompleteResultWrapper
 * The session name is stored as an FName
 * This struct is used to store the result of the OnJoinSessionComplete delegate
 * It is necessary because the OnJoinSessionComplete delegate returns the result of the join session operation
 * and the session name separately
 * We need to store the result and the session name together
 * So we use this struct to store the result and the session name together
 */
USTRUCT(BlueprintType)
struct FJoinSessionResultWrapper
{
	GENERATED_BODY()

	FJoinSessionResultWrapper()
		: Result(EOnJoinSessionCompleteResultWrapper::UnknownError) // Initialize Result with a default value
	{
	}

	UPROPERTY(BlueprintReadOnly)
	EOnJoinSessionCompleteResultWrapper Result;

	UPROPERTY(BlueprintReadOnly)
	FString Address;
};

/**
 * This delegate is used to broadcast when the CreateSession operation is complete
 * It is used to notify the menu that the CreateSession operation is complete
 * The menu can bind a callback to this delegate to be notified when the CreateSession operation is complete
 * The delegate takes a single parameter, a boolean, which is true if the operation was successful and false if it was not
 * The menu can bind a callback to this delegate to be notified when the CreateSession operation is complete
 * The callback will be called with a single parameter, a boolean, which is true if the operation was successful and false if it was not
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWasSuccessful);
/**
 * This delegate is used to broadcast when the FindSessions operation is complete
 * It is used to notify the menu that the FindSessions operation is complete
 * The menu can bind a callback to this delegate to be notified when the FindSessions operation is complete
 * The delegate takes two parameters, an FOnlineSessionSearchResultWrapper and a boolean
 * The FOnlineSessionSearchResultWrapper contains the results of the search
 * The boolean is true if the operation was successful and false if it was not
 * The menu can bind a callback to this delegate to be notified when the FindSessions operation is complete
 * The callback will be called with two parameters, an FOnlineSessionSearchResultWrapper and a boolean
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete, FOnlineSessionSearchResultWrapper, SessionResultsWrapper, bool, bWasSuccessful);
/**
 * This delegate is used to broadcast when the JoinSession operation is complete
 * It is used to notify the menu that the JoinSession operation is complete
 * The menu can bind a callback to this delegate to be notified when the JoinSession operation is complete
 * The delegate takes a single parameter, an FJoinSessionResultWrapper
 * The FJoinSessionResultWrapper contains the result of the join session operation
 * The menu can bind a callback to this delegate to be notified when the JoinSession operation is complete
 * The callback will be called with a single parameter, an FJoinSessionResultWrapper
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete, FJoinSessionResultWrapper, ResultWrapper);
/**
 * This delegate is used to broadcast when the DestroySession operation is complete
 * It is used to notify the menu that the DestroySession operation is complete
 * The menu can bind a callback to this delegate to be notified when the DestroySession operation is complete
 * The delegate takes a single parameter, a boolean, which is true if the operation was successful and false if it was not
 * The menu can bind a callback to this delegate to be notified when the DestroySession operation is complete
 * The callback will be called with a single parameter, a boolean, which is true if the operation was successful and false if it was not
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete, bool, bWasSuccessful);
/**
 * This delegate is used to broadcast when the StartSession operation is complete
 * It is used to notify the menu that the StartSession operation is complete
 * The menu can bind a callback to this delegate to be notified when the StartSession operation is complete
 * The delegate takes a single parameter, a boolean, which is true if the operation was successful and false if it was not
 * The menu can bind a callback to this delegate to be notified when the StartSession operation is complete
 * The callback will be called with a single parameter, a boolean, which is true if the operation was successful and false if it was not
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnServerListRefreshedCompleteDelegate, bool, bWasSuccessful);

/**
 * This class is used to handle session functionality
 * It is a GameInstanceSubsystem, so it is created and destroyed with the GameInstance
 * It is used to create, find, join, destroy, and start sessions
 * The Menu will call the functions in this class to handle session functionality
 * The class contains delegates that the Menu can bind callbacks to
 * The class contains internal callbacks that are bound to the Online Session Interface delegates
 * The internal callbacks are called when the Online Session Interface delegates are called
 * The internal callbacks broadcast the results of the session operations to the Menu
 * The Menu can bind callbacks to the delegates to be notified when the session operations are complete
 */
UCLASS()
class STEAMMULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem, public IMenuInterface
{
	GENERATED_BODY()

public:
	/**
	 * Default constructor
	 * This constructor initializes the SessionInterface
	 * The SessionInterface is used to interact with the Online Session Interface
	 */
	UMultiplayerSessionsSubsystem();

	/**
	 * This function is used to load a menu widget to the viewport
	 * I.e. the main menu, the server browser, the in-game menu, etc.
	 * @param MenuWidget The menu widget to load
	 */
	UFUNCTION(BlueprintCallable)
	void LoadMenuWidget(UUserWidget *MenuWidget) override;

	virtual void Host(const FString ServerName = TEXT("Ivan's Test Session")) override;

	// UFUNCTION(Exec)
	virtual void Join(uint32 index) override;
	// virtual void Join(const FString& SessionName = FString("FreeForAll")) ;

	/**
	 * This function is used to create a session
	 * @param NumPublicConnections The number of public connections the session will have
	 * @param MatchType The type of match the session will be
	 */
	UFUNCTION(BlueprintCallable)
	void CreateSession(int32 NumPublicConnections = 4);
	/**
	 * This function is used to find sessions
	 * @param MaxSearchResults The maximum number of search results to return
	 */
	UFUNCTION(BlueprintCallable)
	void FindSessions(int32 MaxSearchResults = 10000);
	/**
	 * This function is used to join a session
	 * @param SessionResultWrapper The session to join
	 */
	UFUNCTION(BlueprintCallable)
	void JoinSession();
	/**
	 * This function is used to destroy a session
	 * It will destroy the current session
	 */
	UFUNCTION(BlueprintCallable)
	void DestroySession();
	/**
	 * This function is used to start the session
	 * It will start the current session
	 */
	UFUNCTION(BlueprintCallable)
	void StartSession();

	/**
	 * This delegate is used to broadcast when the CreateSession operation is complete
	 * It is used to notify the menu that the CreateSession operation is complete
	 * The menu can bind a callback to this delegate to be notified when the CreateSession operation is complete
	 * The delegate takes a single parameter, a boolean, which is true if the operation was successful and false if it was not
	 */
	UPROPERTY(BlueprintAssignable)
	FMultiplayerOnCreateSessionComplete MultiplayerOnCreateSessionComplete;
	/**
	 * This delegate is used to broadcast when the FindSessions operation is complete
	 * It is used to notify the menu that the FindSessions operation is complete
	 * The menu can bind a callback to this delegate to be notified when the FindSessions operation is complete
	 * The delegate takes two parameters, an FOnlineSessionSearchResultWrapper and a boolean
	 * The FOnlineSessionSearchResultWrapper contains the results of the search
	 * The boolean is true if the operation was successful and false if it was not
	 */
	FMultiplayerOnFindSessionsComplete MultiplayerOnFindSessionsComplete;
	/**
	 * This delegate is used to broadcast when the JoinSession operation is complete
	 * It is used to notify the menu that the JoinSession operation is complete
	 * The menu can bind a callback to this delegate to be notified when the JoinSession operation is complete
	 * The delegate takes a single parameter, an FJoinSessionResultWrapper
	 * The FJoinSessionResultWrapper contains the result of the join session operation
	 */
	FMultiplayerOnJoinSessionComplete MultiplayerOnJoinSessionComplete;
	/**
	 * This delegate is used to broadcast when the DestroySession operation is complete
	 * It is used to notify the menu that the DestroySession operation is complete
	 * The menu can bind a callback to this delegate to be notified when the DestroySession operation is complete
	 * The delegate takes a single parameter, a boolean, which is true if the operation was successful and false if it was not
	 */
	FMultiplayerOnDestroySessionComplete MultiplayerOnDestroySessionComplete;
	/**
	 * This delegate is used to broadcast when the StartSession operation is complete
	 * It is used to notify the menu that the StartSession operation is complete
	 * The menu can bind a callback to this delegate to be notified when the StartSession operation is complete
	 * The delegate takes a single parameter, a boolean, which is true if the operation was successful and false if it was not
	 */
	FMultiplayerOnStartSessionComplete MultiplayerOnStartSessionComplete;

	// FOnServerListRefreshedCompleteDelegate MultiplsyerOnServerListRefreshedComplete;

	// FOnServerListRefreshedComplete OnServerListRefreshedComplete;

	// static function to convert the EOnJoinSessionCompleteResult::Type to our own custom EOnJoinSessionCompleteResultWrapper
	static EOnJoinSessionCompleteResultWrapper ConvertEOnJoinSessionCompleteResultToWrapper(EOnJoinSessionCompleteResult::Type Result);

	// static function to convert EOnJoinSessionCompleteResultWrapper to EOnJoinSessionCompleteResult::Type
	static EOnJoinSessionCompleteResult::Type ConvertEOnJoinSessionCompleteResultWrapperToType(EOnJoinSessionCompleteResultWrapper ResultWrapper);

	/**
	 *	This refreshes the server list by calling FindSessions
	 */
	UFUNCTION(BlueprintCallable)
	void RefreshServerList() override;

	/*UFUNCTION(BlueprintCallable)
	void OnRefreshSessionsComplete(bool bWasSuccessful);*/

protected:
	/**
	 * This function is called when the CreateSession operation is complete
	 * It is an internal callback that is bound to the Online Session Interface delegate
	 * It broadcasts the result of the CreateSession operation to the Menu
	 * @param SessionName The name of the session that was created
	 * @param bWasSuccessful True if the operation was successful, false if it was not
	 */
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	/**
	 * This function is called when the FindSessions operation is complete
	 * It is an internal callback that is bound to the Online Session Interface delegate
	 * It broadcasts the results of the FindSessions operation to the Menu
	 * @param bWasSuccessful True if the operation was successful, false if it was not
	 */
	void OnFindSessionsComplete(bool bWasSuccessful);
	/**
	 * This function is called when the JoinSession operation is complete
	 * It is an internal callback that is bound to the Online Session Interface delegate
	 * It broadcasts the result of the JoinSession operation to the Menu
	 * @param SessionName The name of the session that was joined
	 * @param Result The result of the join session operation
	 */
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	/**
	 * This function is called when the DestroySession operation is complete
	 * It is an internal callback that is bound to the Online Session Interface delegate
	 * It broadcasts the result of the DestroySession operation to the Menu
	 * @param SessionName The name of the session that was destroyed
	 * @param bWasSuccessful True if the operation was successful, false if it was not
	 */
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	/**
	 * This function is called when the StartSession operation is complete
	 * It is an internal callback that is bound to the Online Session Interface delegate
	 * It broadcasts the result of the StartSession operation to the Menu
	 * @param SessionName The name of the session that was started
	 * @param bWasSuccessful True if the operation was successful, false if it was not
	 */
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);

	/**
	 * This function is called when the Server List Refresh is complete
	 * It is an internal callback that is bound to the Online Session Interface delegate
	 * It broadcasts the result of the Server List Refresh to the Menu
	 * @param bWasSuccessful True if the operation was successful, false if it was not
	 */
	/*void OnServerListRefreshedComplete(bool bWasSuccessful);*/

	/**
	 * This function is called when the subsystem is initialized
	 * It is an override of the Initialize function in UGameInstanceSubsystem
	 * It initializes the Online Session Interface
	 * @param Collection The subsystem collection
	 */
	virtual void Initialize(FSubsystemCollectionBase &Collection) override;

	virtual void Deinitialize() override;

private:
	// Create Online Session pointer called SessionInterface
	IOnlineSessionPtr SessionInterface;

	// Create Online Session Settings pointer called SessionSettings
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	// Create Online Session Search pointer called LastSessionSearch
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

	// Create FOnlineSessionSearchResultWrapper called OnlineSessionSearchResultWrapper
	FOnlineSessionSearchResultWrapper OnlineSessionSearchResultWrapper;

	FJoinSessionResultWrapper JoinSessionResultWrapper;

	//
	// To addto the Online Session Interface delegate list.
	// We'll bind our MulitplayerSessionSubsystem internal callbacks to these delegates
	//
	// Create CreateSessionCompleteDelegate
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	// delegate handle for create session complete
	FDelegateHandle CreateSessionCompleteDelegateHandle;

	// create FindSessionCompleteDelegate
	FOnFindSessionsCompleteDelegate FindSessionCompleteDelegate;
	// delegate handle for find session complete
	FDelegateHandle FindSessionCompleteDelegateHandle;

	// create JoinSessionCompleteDelegate
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	// delegate handle for join session complete
	FDelegateHandle JoinSessionCompleteDelegateHandle;

	// create DestroySessionCompleteDelegate
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	// delegate handle for destroy session complete
	FDelegateHandle DestroySessionCompleteDelegateHandle;

	// create OnStartSessionCompleteDelegate
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	// delegate handle for start session complete
	FDelegateHandle StartSessionCompleteDelegateHandle;

	// FOnServerListRefreshedCompleteDelegate ServerListRefreshedCompleteDelegate;
	// FDelegateHandle ServerListRefreshedCompleteDelegateHandle;

	/*FOnServerListRefreshedCompleteDelegate ServerListRefreshedCompleteDelegate;*/

	/**
	 * This function is used to clean up the session after a failed join session operation
	 * It is called when the JoinSession operation fails
	 */
	void FailedJoinSessionCleanUp();

	// Create a variable to store the desired server name
	FString DesiredServerName;

	// Create a variable to store a UUserWidget
	TObjectPtr<UUserWidget> WidgetToLoad;

	// Array of menu classes to load
	TArray<TSubclassOf<UUserWidget>> MenuClass;

	// this wrapper function is used to call the Host function from the console command
	void HostWrapper(const TArray<FString> &Args);

	// this wrapper function is used to call the Join function from the console command
	// void JoinWrapper(const TArray<FString>& Args);
};
