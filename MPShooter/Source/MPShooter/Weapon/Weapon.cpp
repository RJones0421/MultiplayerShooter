// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "MPShooter/Character/BlasterCharacter.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>( TEXT( "WeaponMesh" ) );
	WeaponMesh->SetupAttachment( RootComponent );
	SetRootComponent( WeaponMesh );

	// Set collision for when item is dropped/interactable
	WeaponMesh->SetCollisionResponseToAllChannels( ECollisionResponse::ECR_Block );
	WeaponMesh->SetCollisionResponseToChannel( ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore );
	WeaponMesh->SetCollisionEnabled( ECollisionEnabled::NoCollision );

	// Setup and disable on client to let the server handle
	AreaSphere = CreateDefaultSubobject<USphereComponent>( TEXT( "AreaSphere" ) );
	AreaSphere->SetupAttachment( RootComponent );
	AreaSphere->SetCollisionResponseToAllChannels( ECollisionResponse::ECR_Ignore );
	AreaSphere->SetCollisionEnabled( ECollisionEnabled::NoCollision );

	// Pickup widget for weapon
	PickupWidget = CreateDefaultSubobject<UWidgetComponent>( TEXT( "PickupWidget" ) );
	PickupWidget->SetupAttachment( RootComponent );
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	if ( HasAuthority() )
	{
		AreaSphere->SetCollisionEnabled( ECollisionEnabled::QueryAndPhysics );
		AreaSphere->SetCollisionResponseToChannel( ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap );
		AreaSphere->OnComponentBeginOverlap.AddDynamic( this, &AWeapon::OnSphereOverlap );
		AreaSphere->OnComponentEndOverlap.AddDynamic( this, &AWeapon::OnSphereEndOverlap );
	}

	if ( PickupWidget )
	{
		PickupWidget->SetVisibility( false );
	}
}

void AWeapon::GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );

	DOREPLIFETIME( AWeapon, WeaponState );
}

// Called every frame
void AWeapon::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

void AWeapon::OnSphereOverlap( UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult )
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>( OtherActor );

	if ( BlasterCharacter && PickupWidget )
	{
		BlasterCharacter->SetOverlappingWeapon( this );
	}
}

void AWeapon::OnSphereEndOverlap( UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex )
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>( OtherActor );

	if ( BlasterCharacter && PickupWidget )
	{
		BlasterCharacter->SetOverlappingWeapon( nullptr );
	}
}

void AWeapon::ShowPickupWidget( bool bShowWidget )
{
	if ( PickupWidget )
	{
		PickupWidget->SetVisibility( bShowWidget );
	}
}

void AWeapon::OnRep_WeaponState()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget( false );
		break;
	}
}

void AWeapon::SetWeaponState( EWeaponState State )
{
	WeaponState = State;
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget( false );
		AreaSphere->SetCollisionEnabled( ECollisionEnabled::NoCollision );
		break;
	}
}
