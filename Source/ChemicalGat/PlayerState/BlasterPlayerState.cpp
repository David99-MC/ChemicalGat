// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerState.h"
#include "ChemicalGat/Character/BlasterCharacter.h"
#include "ChemicalGat/PlayerController/BlasterPlayerController.h"
#include "Net/UnrealNetwork.h"

void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ABlasterPlayerState, Defeat);
}

void ABlasterPlayerState::AddToScore(float ScoreAmount)
{
    float NewScore = GetScore() + ScoreAmount;
    SetScore(NewScore);
    OnScoreUpdate();
}

void ABlasterPlayerState::OnRep_Score()
{
    Super::OnRep_Score();

    OnScoreUpdate();
}

void ABlasterPlayerState::OnScoreUpdate()
{
    if (!IsControllerValid())
        return;

    // Update the HUD through player controller
    BlasterController->SetHUDScore(GetScore());
}

void ABlasterPlayerState::AddToDefeat(int32 DefeateAmount)
{
    Defeat += DefeateAmount;
    OnDefeatUpdate();
}

void ABlasterPlayerState::OnRep_Defeat()
{
    OnDefeatUpdate();
}

void ABlasterPlayerState::OnDefeatUpdate()
{
    if (!IsControllerValid())
        return;

    BlasterController->SetHUDDefeat(Defeat);
}

bool ABlasterPlayerState::IsControllerValid()
{
    BlasterCharacter = BlasterCharacter == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : BlasterCharacter;
    if (BlasterCharacter)
        BlasterController = BlasterController == nullptr ? Cast<ABlasterPlayerController>(BlasterCharacter->Controller) : BlasterController;

    return BlasterCharacter && BlasterController;
}
