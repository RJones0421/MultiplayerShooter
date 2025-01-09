// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

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

// Called to bind functionality to input
void ABlasterCharacter::SetupPlayerInputComponent( UInputComponent* PlayerInputComponent )
{
	Super::SetupPlayerInputComponent( PlayerInputComponent );

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>( PlayerInputComponent ))
	{
		EnhancedInputComponent->BindAction( JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump );

		EnhancedInputComponent->BindAction( MoveForwardAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::MoveForward );
		EnhancedInputComponent->BindAction( MoveRightAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::MoveRight );
		EnhancedInputComponent->BindAction( TurnAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::Turn );
		EnhancedInputComponent->BindAction( LookUpAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::LookUp );
	}
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

// ----- Updates ----------------------------------------------------------------------------------

// Called every frame
void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

