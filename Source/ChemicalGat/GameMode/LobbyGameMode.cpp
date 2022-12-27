// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    int32 Players = GameState.Get()->PlayerArray.Num();
    if (Players == 2)
    {
        if (UWorld* World = GetWorld())
        {
            bUseSeamlessTravel = true;
            World->ServerTravel(FString("/Game/Maps/ScifiCity_Level?lister"));
        }
    }
}