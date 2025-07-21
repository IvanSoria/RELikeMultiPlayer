// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MenuWidget.h"
#include "MainMenu.generated.h"

class UButton;
class UWidgetSwitcher;
class UWidget;
class UEditableTextBox;
class UPanelWidget;
class UUserWidget;

USTRUCT()
struct FServerData
{
	GENERATED_BODY()

	FString Name;
	uint16 CurrentPlayers;
	uint16 MaxPlayers;
	FString HostUserName;
};

/**
 * 
 */
UCLASS()
class STEAMMULTIPLAYERSESSIONS_API UMainMenu : public UMenuWidget
{
	GENERATED_BODY()

public:
	UMainMenu(const FObjectInitializer& ObjectInitializer);

	void SetServerList(TArray<FServerData> ServerNames);

	void SelectIndex(uint32 Index);

	/* UFUNCTION() Called when the session is created
	 * This will call the appropriate function to handle the creation of the session
	 * @param bWasSuccessful - true if the session was created successfully, false otherwise
	*/
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);

	/**
	 * UFUNCTION() called when the session is destroyed
	 * This will call the appropriate function to handle the destruction of the session
	 * @param bWasSuccessful - true if the session was destroyed successfully, false otherwise
	*/
	UFUNCTION()
	void OnDestroySessionComplete(bool bWasSuccessful);

	/**
	 * UFUNCTION() called when the session is started
	 * This will call the appropriate function to handle the starting of the session
	 * @param bWasSuccessful - true if the session was started successfully, false otherwise
	*/
	UFUNCTION()
	void OnStartSessionComplete(bool bWasSuccessful);

	/**
	 * UFUNCTION() called when the session is joined
	 * This will call the appropriate function to handle the joining of the session
	 * @param JoinSessionResult - the result of the join session operation
	*/
	UFUNCTION()
	void OnJoinSessionComplete(FJoinSessionResultWrapper JoinSessionResult);


protected:
	virtual bool Initialize() override;

private:
	TSubclassOf<class UUserWidget> ServerRowClass;

	UPROPERTY(meta = (BindWidget))
		class UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
		class UButton* JoinButton;

	UPROPERTY(meta = (BindWidget))
		class UButton* QuitButton;

	UPROPERTY(meta = (BindWidget))
		class UButton* CancelJoinMenuButton;

	UPROPERTY(meta = (BindWidget))
		class UButton* ConfirmJoinMenuButton;

	UPROPERTY(meta = (BindWidget))
		class UWidgetSwitcher* MenuSwitcher;

	UPROPERTY(meta = (BindWidget))
		class UWidget* MainMenu;

	UPROPERTY(meta = (BindWidget))
		class UWidget* JoinMenu;

	UPROPERTY(meta = (BindWidget))
		class UWidget* HostMenu;

	UPROPERTY(meta = (BindWidget))
		class UEditableTextBox* ServerHostName;

	UPROPERTY(meta = (BindWidget))
		class UButton* CancelHostMenuButton;

	UPROPERTY(meta = (BindWidget))
		class UButton* ConfirmHostMenuButton;

	UPROPERTY(meta = (BindWidget))
		class UPanelWidget* ServerList;

	UFUNCTION()
		void HostServer();

	UFUNCTION()
		void JoinServer();


	UFUNCTION()
		void OpenHostMenu();

	UFUNCTION()
		void OpenJoinMenu();

	UFUNCTION()
		void OpenMainMenu();

	UFUNCTION()
		void QuitPressed();

	TOptional<uint32> SelectedIndex;

	void UpdateChildren();

	//the subsystem we will use to handle session functionality
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

};
