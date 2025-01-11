// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "BlasterCharacter.generated.h"

class AWeapon;
class UCameraComponent;
class UCombatComponent;
class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UWidgetComponent;

UCLASS()
class MPSHOOTER_API ABlasterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	virtual void PostInitializeComponents() override;
	virtual void Tick( float DeltaTime ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent( class UInputComponent* PlayerInputComponent ) override;
	virtual void GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const override;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward( const FInputActionValue& Value );
	void MoveRight( const FInputActionValue& Value );
	void Turn( const FInputActionValue& Value );
	void LookUp( const FInputActionValue& Value );
	void EquipButtonPressed( const FInputActionValue& Value );

private:
	UFUNCTION()
	void OnRep_OverlappingWeapon( AWeapon* LastWeapon );

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* PlayerMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* JumpAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* MoveForwardAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* MoveRightAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* TurnAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* LookUpAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* EquipAction;

private:
	// Unreal functions
	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	// Unreal properties
	UPROPERTY(VisibleAnywhere, Category = Camera)
	USpringArmComponent* CameraBoom;

	UPROPERTY( VisibleAnywhere, Category = Camera )
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;

	UPROPERTY(VisibleAnywhere)
	UCombatComponent* Combat;


public:
	void SetOverlappingWeapon( AWeapon* Weapon );
};
