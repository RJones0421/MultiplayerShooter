// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "MPShooter/Character/BlasterCharacter.h"
#include "MPShooter/Weapon/Weapon.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCombatComponent::GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );

	DOREPLIFETIME( UCombatComponent, EquippedWeapon );
}

void UCombatComponent::EquipWeapon( AWeapon* WeaponToEquip )
{
	// Verify pointers
	if (!Character)
	{
		return;
	}
	if (!WeaponToEquip)
	{
		return;
	}

	// Set weapon and display it
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState( EWeaponState::EWS_Equipped );
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName( FName( "RightHandSocket" ) );
	if (HandSocket)
	{
		HandSocket->AttachActor( EquippedWeapon, Character->GetMesh() );
	}
	EquippedWeapon->SetOwner( Character );
}

