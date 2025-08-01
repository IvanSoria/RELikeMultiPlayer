// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ServerRow.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class STEAMMULTIPLAYERSESSIONS_API UServerRow : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ServerName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HostUser;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ConnectionFraction;

	UPROPERTY(BlueprintReadOnly)
	bool Selected = false;

	void Setup(class UMainMenu* Parent, uint32 Index);

private:
	UPROPERTY(meta = (BindWidget))
	class UButton* RowButton;

	UPROPERTY()
	class UMainMenu* Parent;

	UPROPERTY( VisibleAnywhere, meta = ( AllowPrivateAccess ))
	uint32 Index;

	UFUNCTION()
	void OnClicked();
};
