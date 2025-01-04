// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem()
  : CreateSessionCompleteDelegate ( FOnCreateSessionCompleteDelegate::CreateUObject( this, &ThisClass::OnCreateSessionComplete ) )
  , FindSessionsCompleteDelegate  ( FOnFindSessionsCompleteDelegate::CreateUObject( this, &ThisClass::OnFindSessionsComplete ) )
  , JoinSessionCompleteDelegate   ( FOnJoinSessionCompleteDelegate::CreateUObject( this, &ThisClass::OnJoinSessionComplete ) )
  , DestroySessionCompleteDelegate( FOnDestroySessionCompleteDelegate::CreateUObject( this, &ThisClass::OnDestroySessionComplete ) )
  , StartSessionCompleteDelegate  ( FOnStartSessionCompleteDelegate::CreateUObject( this, &ThisClass::OnStartSessionComplete ) )
{
  IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
  if (Subsystem)
  {
    SessionInterface = Subsystem->GetSessionInterface();
  }
}


/**
 * Public functions to be used with menus to handle functionality
 */
void UMultiplayerSessionsSubsystem::CreateSession( int32 NumPublicConnections, FString MatchType )
{
  // Verify session interface
  if (!SessionInterface.IsValid())
  {
    return;
  }

  // Get rid of old session if applicable
  auto ExistingSession = SessionInterface->GetNamedSession( NAME_GameSession );
  if (ExistingSession)
  {
    SessionInterface->DestroySession( NAME_GameSession );
  }

  // Store the handle to later remove it from the list
  CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle( CreateSessionCompleteDelegate );

  // Lobby settings
  LastSessionSettings = MakeShareable( new FOnlineSessionSettings() );
  LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
  LastSessionSettings->NumPublicConnections = NumPublicConnections;
  LastSessionSettings->bAllowJoinInProgress = true;
  LastSessionSettings->bAllowJoinViaPresence = true;
  LastSessionSettings->bShouldAdvertise = true;
  LastSessionSettings->bUsesPresence = true;
  LastSessionSettings->bUseLobbiesIfAvailable = true;
  LastSessionSettings->Set( FName( "MatchType" ), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing );

  // Create session
  const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
  if (!SessionInterface->CreateSession( *LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings ))
  {
    SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle( CreateSessionCompleteDelegateHandle );

    // Broadcast custom delegate
    MultiplayerOnCreateSessionComplete.Broadcast( false );
  }
}

void UMultiplayerSessionsSubsystem::FindSessions( int32 MaxSearchResults )
{
  if (!SessionInterface.IsValid())
  {
    return;
  }

  FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle( FindSessionsCompleteDelegate );

  LastSessionSearch = MakeShareable( new FOnlineSessionSearch() );
  LastSessionSearch->MaxSearchResults = MaxSearchResults;
  LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
  LastSessionSearch->QuerySettings.Set( SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals );

  const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
  if (SessionInterface->FindSessions( *LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef() ))
  {
    SessionInterface->ClearOnCancelFindSessionsCompleteDelegate_Handle( FindSessionsCompleteDelegateHandle );

    MultiplayerOnFindSessionsComplete.Broadcast( TArray<FOnlineSessionSearchResult>(), false );
  }
}

void UMultiplayerSessionsSubsystem::JoinSession( const FOnlineSessionSearchResult& SessionResult )
{
  if (!SessionInterface.IsValid())
  {
    MultiplayerOnJoinSessionComplete.Broadcast( EOnJoinSessionCompleteResult::UnknownError );
    return;
  }

  JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle( JoinSessionCompleteDelegate );

  // Join session and handle failure
  const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
  if (!SessionInterface->JoinSession( *LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult ))
  {
    SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle( JoinSessionCompleteDelegateHandle );

    MultiplayerOnJoinSessionComplete.Broadcast( EOnJoinSessionCompleteResult::UnknownError );
  }
}

void UMultiplayerSessionsSubsystem::DestroySession()
{
}

void UMultiplayerSessionsSubsystem::StartSession()
{
}

/**
 * Internal functions to be used as callbacks for the online session delegates
 */
void UMultiplayerSessionsSubsystem::OnCreateSessionComplete( FName SessionName, bool bWasSuccessful )
{
  if (SessionInterface)
  {
    SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle( CreateSessionCompleteDelegateHandle );
  }

  MultiplayerOnCreateSessionComplete.Broadcast( true );
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete( bool bWasSuccessful )
{
  // Clear handle
  if (SessionInterface)
  {
    SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle( FindSessionsCompleteDelegateHandle );
  }

  // If no results, return failed search
  if (LastSessionSearch->SearchResults.Num() <= 0)
  {
    MultiplayerOnFindSessionsComplete.Broadcast( TArray<FOnlineSessionSearchResult>(), false );
    return;
  }

  // Send message
  MultiplayerOnFindSessionsComplete.Broadcast( LastSessionSearch->SearchResults, bWasSuccessful );
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete( FName SessionName, EOnJoinSessionCompleteResult::Type Result )
{
  // Clear handle
  if (SessionInterface)
  {
    SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle( JoinSessionCompleteDelegateHandle );
  }

  // Send message
  MultiplayerOnJoinSessionComplete.Broadcast( Result );
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete( FName SessionName, bool bWasSuccessful )
{
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete( FName SessionName, bool bWasSuccessful )
{
}
