// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "MPShooter/BlasterComponents/CombatComponent.h"
#include "MPShooter/Weapon/Weapon.h"

// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>( TEXT( "CameraBoom" ) );
	CameraBoom->SetupAttachment( GetMesh() );
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>( TEXT( "FollowCamera" ) );
	FollowCamera->SetupAttachment( CameraBoom, USpringArmComponent::SocketName );
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>( TEXT( "OverheadWidget" ) );
	OverheadWidget->SetupAttachment( RootComponent );

	Combat = CreateDefaultSubobject<UCombatComponent>( TEXT( "CombatComponent" ) );
	Combat->SetIsReplicated( true );
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Combat)
	{
		Combat->Character = this;
	}
}

// Called when the game starts or when spawned
void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (APlayerController* PlayerController = Cast<APlayerController>( GetController() ))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>( PlayerController->GetLocalPlayer() ))
		{
			Subsystem->AddMappingContext( PlayerMappingContext, 0 );
		}
	}
}

// Called every frame
void ABlasterCharacter::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
}

// Called to bind functionality to input
void ABlasterCharacter::SetupPlayerInputComponent( UInputComponent* PlayerInputComponent )
{
	Super::SetupPlayerInputComponent( PlayerInputComponent );

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>( PlayerInputComponent ))
	{
		EnhancedInputComponent->BindAction( JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump );
		EnhancedInputComponent->BindAction( JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping );
		EnhancedInputComponent->BindAction( EquipAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::EquipButtonPressed );

		EnhancedInputComponent->BindAction( MoveForwardAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::MoveForward );
		EnhancedInputComponent->BindAction( MoveRightAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::MoveRight );
		EnhancedInputComponent->BindAction( TurnAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::Turn );
		EnhancedInputComponent->BindAction( LookUpAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::LookUp );
	}
}

void ABlasterCharacter::GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );

	DOREPLIFETIME_CONDITION( ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly );
}

// ----- Controls ---------------------------------------------------------------------------------

void ABlasterCharacter::MoveForward( const FInputActionValue& Value )
{
	const float CurrentValue = Value.Get<float>();

	if (Controller && CurrentValue != 0.f)
	{
		// Find the camera direction
		const FRotator YawRotation( 0.f, Controller->GetControlRotation().Yaw, 0.f );
		const FVector Direction( FRotationMatrix( YawRotation ).GetUnitAxis( EAxis::X ) );

		// Move
		AddMovementInput( Direction, CurrentValue );
	}
}

void ABlasterCharacter::MoveRight( const FInputActionValue& Value )
{
	const float CurrentValue = Value.Get<float>();

	if (Controller && CurrentValue != 0.f)
	{
		// Find the camera direction
		const FRotator YawRotation( 0.f, Controller->GetControlRotation().Yaw, 0.f );
		const FVector Direction( FRotationMatrix( YawRotation ).GetUnitAxis( EAxis::Y ) );

		// Move
		AddMovementInput( Direction, CurrentValue );
	}
}

void ABlasterCharacter::Turn( const FInputActionValue& Value )
{
	const float CurrentValue = Value.Get<float>();

	AddControllerYawInput( CurrentValue );
}

void ABlasterCharacter::LookUp( const FInputActionValue& Value )
{
	const float CurrentValue = Value.Get<float>();

	AddControllerPitchInput( CurrentValue );
}

void ABlasterCharacter::EquipButtonPressed( const FInputActionValue& Value )
{
	if (Combat && HasAuthority())
	{
		Combat->EquipWeapon( OverlappingWeapon );
	}
}

// ----- Replication ------------------------------------------------------------------------------

// Sets the overlapping weapon to be used and shows prompt to server player
void ABlasterCharacter::SetOverlappingWeapon( AWeapon* Weapon )
{
	// Hide the widget if one currently exists
	if ( OverlappingWeapon )
	{
		OverlappingWeapon->ShowPickupWidget( false );
	}

	// Update reference and show if needed
	OverlappingWeapon = Weapon;
	if ( IsLocallyControlled() )
	{
		if ( OverlappingWeapon )
		{
			OverlappingWeapon->ShowPickupWidget( true );
		}
	}
}

// Rep notify to show/hide prompt
void ABlasterCharacter::OnRep_OverlappingWeapon( AWeapon* LastWeapon )
{
	if ( OverlappingWeapon )
	{
		OverlappingWeapon->ShowPickupWidget( true );
	}

	if ( LastWeapon )
	{
		LastWeapon->ShowPickupWidget( false );
	}
}
