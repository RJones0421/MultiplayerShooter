// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

void UMenu::MenuSetup( int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath )
{
  NumPublicConnections = NumberOfPublicConnections;
  MatchType = TypeOfMatch;
  PathToLobby = FString::Printf( TEXT( "%s?listen" ), *LobbyPath );

  // Adding the widget to the viewport
  AddToViewport();
  SetVisibility( ESlateVisibility::Visible );
  bIsFocusable = true;

  UWorld* World = GetWorld();
  if (World)
  {
    // Get the player and have them access the menu
    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (PlayerController)
    {
      FInputModeUIOnly InputModeData;
      InputModeData.SetWidgetToFocus( TakeWidget() );
      InputModeData.SetLockMouseToViewportBehavior( EMouseLockMode::DoNotLock );
      PlayerController->SetInputMode( InputModeData );
      PlayerController->SetShowMouseCursor( true );
    }
  }

  // Assign the multiplayer sessions subsystem for later use
  UGameInstance* GameInstance = GetGameInstance();
  if (GameInstance)
  {
    MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
  }

  // Bind the callback
  if (MultiplayerSessionsSubsystem)
  {
    MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic( this, &ThisClass::OnCreateSession );
    MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject( this, &ThisClass::OnFindSessions );
    MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject( this, &ThisClass::OnJoinSession );
    MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic( this, &ThisClass::OnDestroySession );
    MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic( this, &ThisClass::OnStartSession );
  }
}

bool UMenu::Initialize()
{
  if (!Super::Initialize())
  {
    return false;
  }

  // Bind the host button if it exists
  if (HostButton)
  {
    HostButton->OnClicked.AddDynamic( this, &ThisClass::HostButtonClicked );
  }

  // Bind the join button if it exists
  if (JoinButton)
  {
    JoinButton->OnClicked.AddDynamic( this, &ThisClass::JoinButtonClicked );
  }

  return true;
}

void UMenu::NativeDestruct()
{
  MenuTeardown();

  Super::NativeDestruct();
}

void UMenu::OnCreateSession( bool bWasSuccessful )
{
  if (bWasSuccessful)
  {
    if (GEngine)
    {
      GEngine->AddOnScreenDebugMessage(
        -1,
        15.f,
        FColor::Yellow,
        FString( "Session created successfully" )
      );
    }

    UWorld* World = GetWorld();
    if (World)
    {
      World->ServerTravel( PathToLobby );
    }
  }
  else
  {
    HostButton->SetIsEnabled( true );

    if (GEngine)
    {
      GEngine->AddOnScreenDebugMessage(
        -1,
        15.f,
        FColor::Red,
        FString( "Session creation failed" )
      );
    }
  }
}

void UMenu::OnFindSessions( const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful )
{
  if (!MultiplayerSessionsSubsystem)
  {
    JoinButton->SetIsEnabled( true );
    return;
  }

  for (auto Result : SessionResults)
  {
    FString SettingsValue;
    Result.Session.SessionSettings.Get( FName( "MatchType" ), SettingsValue );

    if (SettingsValue == MatchType)
    {
      MultiplayerSessionsSubsystem->JoinSession( Result );
      return;
    }
  }

  JoinButton->SetIsEnabled( true );
}

void UMenu::OnJoinSession( EOnJoinSessionCompleteResult::Type Result )
{
  IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
  if (Subsystem)
  {
    IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
    if (SessionInterface.IsValid())
    {
      FString Address;
      SessionInterface->GetResolvedConnectString( NAME_GameSession, Address );

      APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
      if (PlayerController)
      {
        PlayerController->ClientTravel( Address, ETravelType::TRAVEL_Absolute );
      }
    }
  }

  if (Result != EOnJoinSessionCompleteResult::Success)
  {
    JoinButton->SetIsEnabled( true );
  }
}

void UMenu::OnDestroySession( bool bWasSuccessful )
{
}

void UMenu::OnStartSession( bool bWasSuccessful )
{
}

void UMenu::HostButtonClicked()
{
  HostButton->SetIsEnabled( false );

  if (MultiplayerSessionsSubsystem)
  {
    MultiplayerSessionsSubsystem->CreateSession( NumPublicConnections, MatchType );
  }
}

void UMenu::JoinButtonClicked()
{
  JoinButton->SetIsEnabled( false );

  if (MultiplayerSessionsSubsystem)
  {
    MultiplayerSessionsSubsystem->FindSessions( 10000 );
  }
}

void UMenu::MenuTeardown()
{
  RemoveFromParent();
  UWorld* World = GetWorld();
  if (World)
  {
    // Give control back to the player
    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (PlayerController)
    {
      FInputModeGameOnly InputModeData;
      PlayerController->SetInputMode( InputModeData );
      PlayerController->SetShowMouseCursor( false );
    }
  }
}
