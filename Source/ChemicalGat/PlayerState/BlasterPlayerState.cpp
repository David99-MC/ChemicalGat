// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerState.h"
#include "ChemicalGat/Character/BlasterCharacter.h"
#include "ChemicalGat/PlayerController/BlasterPlayerController.h"

void ABlasterPlayerState::AddToScore(float ScoreAmount)
{
    float NewScore = GetScore() + ScoreAmount;
    SetScore(NewScore);

    if (IsControllerValid())
    {
        BlasterController->SetHUDScore(NewScore);
    }
}

void ABlasterPlayerState::OnRep_Score()
{
    Super::OnRep_Score();

    if (IsControllerValid())
    {
        // Update the HUD through player controller
        BlasterController->SetHUDScore(GetScore());
    }
}

bool ABlasterPlayerState::IsControllerValid()
{
    BlasterCharacter = BlasterCharacter == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : BlasterCharacter;
    if (BlasterCharacter)
        BlasterController = BlasterController == nullptr ? Cast<ABlasterPlayerController>(BlasterCharacter->Controller) : BlasterController;

    return BlasterCharacter && BlasterController;
}
