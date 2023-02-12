// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"
#include "ChemicalGat/Character/BlasterCharacter.h"
#include "ChemicalGat/PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "ChemicalGat/PlayerState/BlasterPlayerState.h"

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* EliminatedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
    ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
    ABlasterPlayerState* VictimPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;

    if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
    {
        AttackerPlayerState->AddToScore(1.f);
    }

    if (VictimPlayerState)
    {
        VictimPlayerState->AddToDefeat(1);
    }

    if (EliminatedCharacter )
    {
        EliminatedCharacter->PlayerElim();
        
    }
}

void ABlasterGameMode::RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController)
{
    if (EliminatedCharacter)
    {
        EliminatedCharacter->Reset();
        EliminatedCharacter->Destroy();
    }
    if (EliminatedController)
    {
        TArray<AActor*> PlayerStartLocations;
        UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStartLocations);

        int32 SpawnIndex = FMath::RandRange(0, PlayerStartLocations.Num() - 1);
        
        RestartPlayerAtPlayerStart(EliminatedController, PlayerStartLocations[SpawnIndex]);

    }
}

