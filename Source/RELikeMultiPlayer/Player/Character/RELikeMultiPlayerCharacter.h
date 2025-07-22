// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "RELikeMultiPlayerCharacter.generated.h"

UCLASS(config=Game)
class ARELikeMultiPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

private:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/** Inventory Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UInventoryComponent* InventoryComponent;

	/** Health Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UHealthComponent* HealthComponent;

	/** Stamina Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UStaminaComponent* StaminaComponent;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Input, meta = (AllowPrivateAccess = "true"))
	float TurnRateGamepad;

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<class UPlayerHUDWidget> HUDWidgetClass;

    UPROPERTY()
    class UPlayerHUDWidget* HUDWidget;

    // Add this function
    void SetupHUD();

	// Player state replication
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	// HUD function
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Crouch Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;

	/** Sprint Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	/** Open Inventory Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* OpenInventoryAction;

	/** CrouchEyeOffset */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Crouch, meta = (AllowPrivateAccess = "true"))
	FVector CrouchEyeOffset;

	/** CrouchSpeed */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Crouch, meta = (AllowPrivateAccess = "true"))
	float CrouchSpeed;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for crouch start */
	void CrouchStart();

	/** Called for crouch end */
	void CrouchEnd();

	/** Called for sprint start */
	void SprintStart();

	/** Called for sprint end */
    void SprintEnd();

	/** Called for open inventory */
    void OpenInventory();

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// To add mapping context
	virtual void BeginPlay() override;
	
	// Component lifecycle tracking
	virtual void PostInitializeComponents() override;

	// Replication setup
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<class UUserWidget> InventoryWidgetClass;

    UPROPERTY()
    class UUserWidget* InventoryWidget;

    UPROPERTY(BlueprintReadOnly, Category = "UI")
    bool bIsInventoryOpen = false;

public:
	/** Constructor */
	ARELikeMultiPlayerCharacter();

	/** Tick function */
	virtual void Tick(float DeltaTime) override;

	/** Function to hide the player */
    UFUNCTION(BlueprintCallable, Category="Player")
    void HidePlayer();

    /** Function to show the player */
    UFUNCTION(BlueprintCallable, Category="Player")
    void ShowPlayer();

	/** Functions to control HUD visibility */
	UFUNCTION(BlueprintCallable, Category="HUD")
	void ShowPlayerHUD();

	UFUNCTION(BlueprintCallable, Category="HUD")
	void HidePlayerHUD();

	UFUNCTION(BlueprintCallable, Category="HUD")
	bool IsPlayerHUDVisible() const;

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	
	/** Returns InventoryComponent subobject **/
	FORCEINLINE class UInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

	/** Returns HealthComponent subobject **/
	FORCEINLINE class UHealthComponent* GetHealthComponent() const { return HealthComponent; }

	/** Returns StaminaComponent subobject **/
	FORCEINLINE class UStaminaComponent* GetStaminaComponent() const { return StaminaComponent; }
};

