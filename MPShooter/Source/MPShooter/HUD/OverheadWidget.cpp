// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"
#include "Components/TextBlock.h"

void UOverheadWidget::SetDisplayText( FString TextToDisplay )
{
  if (DisplayText)
  {
    DisplayText->SetText( FText::FromString( TextToDisplay ) );
  }
}

void UOverheadWidget::ShowPlayerNetRole( APawn* InPawn )
{
  ENetRole LocalRole = InPawn->GetLocalRole();
  FString Role;

  switch (LocalRole)
  {
  case ENetRole::ROLE_Authority:
    Role = FString( "Local Role: Authority" );
    break;
  case ENetRole::ROLE_AutonomousProxy:
    Role = FString( "Local Role: AutonomousProxy" );
    break;
  case ENetRole::ROLE_SimulatedProxy:
    Role = FString( "Local Role: SimulatedProxy" );
    break;
  case ENetRole::ROLE_None:
    Role = FString( "Local Role: None" );
    break;
  }

  SetDisplayText( Role );
}

void UOverheadWidget::NativeDestruct()
{
  RemoveFromParent();

  Super::NativeDestruct();
}
