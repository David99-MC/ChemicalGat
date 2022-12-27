// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UOverheadWidget::SetDisplayText(FString TextToDisplay)
{
    if (DisplayText)
    {
        DisplayText->SetText(FText::FromString(TextToDisplay));
    }
}

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
    // if (APlayerState* PlayerState = InPawn->GetPlayerState())
    // {
    //     FString PlayerName = PlayerState->GetPlayerName();
    //     SetDisplayText(PlayerName);
    // }
    ENetRole LocalRole =  InPawn->GetLocalRole();
    switch (LocalRole)
    {
    case ENetRole::ROLE_Authority:
        SetDisplayText(FString("Authority"));
        break;
    case ENetRole::ROLE_AutonomousProxy:
        SetDisplayText(FString("Autonomous Proxy"));
        break;
    case ENetRole::ROLE_SimulatedProxy:
        SetDisplayText(FString("Simulated Proxy"));
        break;
    case ENetRole::ROLE_None:
        SetDisplayText(FString("None"));
        break;
    default:
        break;
    }
}