// Copyright Epic Games, Inc. All Rights Reserved.

#include "RELikeMultiPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/Engine.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "OnlineSessionSettings.h"
#include "../../Components/Inventory/InventoryComponent.h"
#include "../../Components/Health/HealthComponent.h"
#include "../../Components/Stamina/StaminaComponent.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Widget.h"
#include "Net/UnrealNetwork.h"
#include "../../UI/HUD/PlayerHUDWidget.h"


//////////////////////////////////////////////////////////////////////////
// ARELikeMultiPlayerCharacter

ARELikeMultiPlayerCharacter::ARELikeMultiPlayerCharacter() //: 
// 	OnCreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ARELikeMultiPlayerCharacter::OnCreateSessionComplete)),
// 	OnFindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ARELikeMultiPlayerCharacter::OnFindSessionsComplete)),
// 	OnJoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ARELikeMultiPlayerCharacter::OnJoinSessionComplete))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rate for input
	TurnRateGamepad = 50.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true; // Allow the player to crouch

	// Crouch settings to fix animation issues
	GetCharacterMovement()->SetCrouchedHalfHeight(40.0f); // Adjust capsule height when crouching
	GetCharacterMovement()->bCrouchMaintainsBaseLocation = true; // Add this line
	GetCharacterMovement()->MaxWalkSpeedCrouched = 200.0f; // Slower movement when crouched
	// Also set the mesh offset
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f)); // Adjust Z value

	CrouchedEyeHeight = 32.0f;

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	UE_LOG(LogTemp, Log, TEXT("Character Constructor: Creating Components"));

	// Create inventory component
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	if (InventoryComponent)
	{
		UE_LOG(LogTemp, Log, TEXT("InventoryComponent created successfully in constructor"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create InventoryComponent in constructor"));
	}

	// Create health component
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	if (HealthComponent)
	{
		UE_LOG(LogTemp, Log, TEXT("HealthComponent created successfully in constructor"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create HealthComponent in constructor"));
	}

	// Create stamina component
	StaminaComponent = CreateDefaultSubobject<UStaminaComponent>(TEXT("StaminaComponent"));
	if (StaminaComponent)
	{
		UE_LOG(LogTemp, Log, TEXT("StaminaComponent created successfully in constructor"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create StaminaComponent in constructor"));
	}

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	//OnlineSubsytemPtr =  IOnlineSubsystem::Get(); //initialize the pointer to the online subsystem
	
	//set this character to replicate
	SetReplicates(true);
	SetReplicateMovement(true);
	
	// Components automatically replicate with SetIsReplicatedByDefault(true) in their constructors
	// No need to manually call SetIsReplicated here as it's redundant and can cause issues
	UE_LOG(LogTemp, Log, TEXT("Character Constructor: Replication setup completed"));
}

void ARELikeMultiPlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	UE_LOG(LogTemp, Warning, TEXT("=== CHARACTER POST-INITIALIZE COMPONENTS ==="));
	UE_LOG(LogTemp, Log, TEXT("Character PostInitializeComponents: Role = %s"), 
		GetLocalRole() == ROLE_Authority ? TEXT("Authority") : 
		GetLocalRole() == ROLE_AutonomousProxy ? TEXT("AutonomousProxy") : 
		GetLocalRole() == ROLE_SimulatedProxy ? TEXT("SimulatedProxy") : TEXT("None"));
	
	// Validate all components are properly initialized
	bool bAllComponentsValid = true;
	
	if (!HealthComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("✗ HealthComponent is NULL in PostInitializeComponents!"));
		bAllComponentsValid = false;
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("✓ HealthComponent validated in PostInitializeComponents"));
	}
	
	if (!StaminaComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("✗ StaminaComponent is NULL in PostInitializeComponents!"));
		bAllComponentsValid = false;
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("✓ StaminaComponent validated in PostInitializeComponents"));
	}
	
	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("✗ InventoryComponent is NULL in PostInitializeComponents!"));
		bAllComponentsValid = false;
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("✓ InventoryComponent validated in PostInitializeComponents"));
	}
	
	if (bAllComponentsValid)
	{
		UE_LOG(LogTemp, Log, TEXT("✓ All components successfully initialized"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("✗ Some components failed to initialize!"));
	}
	
	UE_LOG(LogTemp, Warning, TEXT("=== END POST-INITIALIZE COMPONENTS ==="));
}

void ARELikeMultiPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// Note: Components automatically handle their own replication when SetIsReplicatedByDefault(true) is called
	// This function is needed for any character-specific properties that need replication
}

void ARELikeMultiPlayerCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("=== CHARACTER BEGINPLAY START ==="));
	UE_LOG(LogTemp, Log, TEXT("Character BeginPlay: Owner Role = %s"), 
		GetLocalRole() == ROLE_Authority ? TEXT("Authority") : 
		GetLocalRole() == ROLE_AutonomousProxy ? TEXT("AutonomousProxy") : 
		GetLocalRole() == ROLE_SimulatedProxy ? TEXT("SimulatedProxy") : TEXT("None"));

	// Debug component initialization with detailed checks
	UE_LOG(LogTemp, Log, TEXT("Character BeginPlay: Verifying Component States"));
	
	if (HealthComponent)
	{
		UE_LOG(LogTemp, Log, TEXT("✓ HealthComponent: Valid (Role: %s, Replicated: %s)"), 
			HealthComponent->GetOwnerRole() == ROLE_Authority ? TEXT("Authority") : TEXT("Non-Authority"),
			HealthComponent->GetIsReplicated() ? TEXT("Yes") : TEXT("No"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("✗ HealthComponent: NULL in BeginPlay!"));
	}

	if (StaminaComponent)
	{
		UE_LOG(LogTemp, Log, TEXT("✓ StaminaComponent: Valid (Role: %s, Replicated: %s)"), 
			StaminaComponent->GetOwnerRole() == ROLE_Authority ? TEXT("Authority") : TEXT("Non-Authority"),
			StaminaComponent->GetIsReplicated() ? TEXT("Yes") : TEXT("No"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("✗ StaminaComponent: NULL in BeginPlay!"));
	}

	if (InventoryComponent)
	{
		UE_LOG(LogTemp, Log, TEXT("✓ InventoryComponent: Valid (Role: %s, Replicated: %s)"), 
			InventoryComponent->GetOwnerRole() == ROLE_Authority ? TEXT("Authority") : TEXT("Non-Authority"),
			InventoryComponent->GetIsReplicated() ? TEXT("Yes") : TEXT("No"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("✗ InventoryComponent: NULL in BeginPlay!"));
	}

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}	

	// Set up HUD after components are verified
    SetupHUD();

	UE_LOG(LogTemp, Warning, TEXT("=== CHARACTER BEGINPLAY END ==="));
}

void ARELikeMultiPlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ARELikeMultiPlayerCharacter::HidePlayer()
{
	// Hide the player
	GetMesh()->SetVisibility(false);
}

void ARELikeMultiPlayerCharacter::ShowPlayer()
{
	// Show the player
	GetMesh()->SetVisibility(true);
}

//////////////////////////////////////////////////////////////////////////
// Input

void ARELikeMultiPlayerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {

		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ARELikeMultiPlayerCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ARELikeMultiPlayerCharacter::Look);

		//Crouching
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ARELikeMultiPlayerCharacter::CrouchStart);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ARELikeMultiPlayerCharacter::CrouchEnd);

		// Sprinting
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &ARELikeMultiPlayerCharacter::SprintStart);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ARELikeMultiPlayerCharacter::SprintEnd);

		// Open Inventory
		EnhancedInputComponent->BindAction(OpenInventoryAction, ETriggerEvent::Started, this, &ARELikeMultiPlayerCharacter::OpenInventory);
			
	}
}




void ARELikeMultiPlayerCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ARELikeMultiPlayerCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ARELikeMultiPlayerCharacter::CrouchStart()
{
	ACharacter::Crouch();


	//check to see GEngine is not a null pointer
	TObjectPtr<UEngine> myEngine = GEngine;
	if (!ensure(myEngine != nullptr)) return;

	myEngine->AddOnScreenDebugMessage(-10, 5.f, FColor::Green, TEXT("Enter CrouchStart"));

	
}

void ARELikeMultiPlayerCharacter::CrouchEnd()
{
	ACharacter::UnCrouch();

	//check to see GEngine is not a null pointer
	TObjectPtr<UEngine> myEngine = GEngine;
	if(!ensure(myEngine != nullptr)) return;

	myEngine->AddOnScreenDebugMessage(-5, 5.f, FColor::Red, TEXT("Enter CrouchEnd"));
	



}

void ARELikeMultiPlayerCharacter::SprintStart()
{
	//check to see GEngine is not a null pointer
	TObjectPtr<UEngine> myEngine = GEngine;
	if(!ensure(myEngine != nullptr)) return;

	// Check if StaminaComponent exists before using it
	if(StaminaComponent && StaminaComponent->CanSprint())
	{
		myEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Sprint Start"));
		StaminaComponent->StartSprinting();
	}
	else if(!StaminaComponent)
	{
		myEngine->AddOnScreenDebugMessage(-2, 5.f, FColor::Red, TEXT("Sprint Start Failed - No Stamina Component"));
	}
}

void ARELikeMultiPlayerCharacter::SprintEnd()
{
	//check to see GEngine is not a null pointer
	TObjectPtr<UEngine> myEngine = GEngine;
	if(!ensure(myEngine != nullptr)) return;

	// Check if StaminaComponent exists before using it
	if(StaminaComponent)
	{
		myEngine->AddOnScreenDebugMessage(-3, 5.f, FColor::Red, TEXT("Sprint End"));
		StaminaComponent->StopSprinting();
	}
	else
	{
		myEngine->AddOnScreenDebugMessage(-4, 5.f, FColor::Red, TEXT("Sprint End Failed - No Stamina Component"));
	}
}

void ARELikeMultiPlayerCharacter::OpenInventory()
{
    // Only local player can open inventory
    if (!IsLocallyControlled()) return;

    APlayerController* PC = Cast<APlayerController>(Controller);
    if (!PC) return;

    if (!bIsInventoryOpen)
    {
        // Create widget if it doesn't exist
        if (!InventoryWidget && InventoryWidgetClass)
        {
            InventoryWidget = CreateWidget<UUserWidget>(PC, InventoryWidgetClass);
        }

        // Show inventory
        if (InventoryWidget)
        {
            InventoryWidget->AddToViewport();
            
            // Set input mode to UI
            FInputModeGameAndUI InputMode;
            InputMode.SetWidgetToFocus(InventoryWidget->TakeWidget());
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            PC->SetInputMode(InputMode);
            PC->bShowMouseCursor = true;
            
            bIsInventoryOpen = true;
        }
    }
    else
    {
        // Hide inventory
        if (InventoryWidget)
        {
            InventoryWidget->RemoveFromParent();
            
            // Set input mode back to game
            FInputModeGameOnly InputMode;
            PC->SetInputMode(InputMode);
            PC->bShowMouseCursor = false;
            
            bIsInventoryOpen = false;
        }
    }
}

void ARELikeMultiPlayerCharacter::SetupHUD()
{
    // Only create HUD for local player
    if (!IsLocallyControlled()) return;
    
    APlayerController* PC = Cast<APlayerController>(Controller);
    if (!PC) return;
    
    // Create HUD widget if we have a class set
    if (HUDWidgetClass && !HUDWidget)
    {
        HUDWidget = CreateWidget<UPlayerHUDWidget>(PC, HUDWidgetClass);
        if (HUDWidget)
        {
            HUDWidget->AddToViewport();
            HUDWidget->SetupPlayerComponents(this);
            
            		UE_LOG(LogTemp, Log, TEXT("HUD Widget created and added to viewport"));
        }
    }
}

// Add these implementations to the cpp file:
void ARELikeMultiPlayerCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    
    // Server-side possession
    SetupHUD();
}

void ARELikeMultiPlayerCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    
    // Client-side possession
    SetupHUD();
}

void ARELikeMultiPlayerCharacter::ShowPlayerHUD()
{
    if (HUDWidget)
    {
        HUDWidget->ShowHUD();
    }
}

void ARELikeMultiPlayerCharacter::HidePlayerHUD()
{
    if (HUDWidget)
    {
        HUDWidget->HideHUD();
    }
}

bool ARELikeMultiPlayerCharacter::IsPlayerHUDVisible() const
{
    if (HUDWidget)
    {
        return HUDWidget->IsHUDVisible();
    }
    return false;
}