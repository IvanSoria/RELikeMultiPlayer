// Fill out your copyright notice in the Description page of Project Settings.


#include "RELikeGameInstance.h"
#include "Engine/Engine.h"

URELikeGameInstance::URELikeGameInstance(const FObjectInitializer& ObjectInitializer)
{
	//Super::Super(ObjectInitializer);
	UE_LOG(LogTemp, Warning, TEXT("GameInstance Constructor"));
}

void  URELikeGameInstance::Init() 
{
	Super::Init();
	UE_LOG(LogTemp, Warning, TEXT("GameInstance Init"));
}

/*void URELikeGameInstance::Host()
{
	TObjectPtr<UEngine> Engine = GetEngine();
	if (!ensure(Engine != nullptr)) return;

	Engine->AddOnScreenDebugMessage(0, 2, FColor::Green,TEXT("Hosting"));

	TObjectPtr<UWorld> World = GetWorld();
	if (!ensure(World != nullptr)) return;

	World->ServerTravel("/Game/ThirdPerson/Maps/ThirdPersonMap?listen");


}

void URELikeGameInstance::Join(const FString& Address)
{
	TObjectPtr<UEngine> Engine = GetEngine();
	if (!ensure(Engine != nullptr)) return;

	Engine->AddOnScreenDebugMessage(0, 5, FColor::Green, FString::Printf(TEXT("Joining %s"), *Address));
	
	TObjectPtr<APlayerController> PlayerController = GetFirstLocalPlayerController();
	if (!ensure(PlayerController != nullptr)) return;

	PlayerController->ClientTravel(Address,ETravelType::TRAVEL_Absolute, true);

}*/
