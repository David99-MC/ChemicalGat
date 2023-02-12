// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"
#include "ChemicalGat/HUD/BlasterHUD.h"
#include "ChemicalGat/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "ChemicalGat/Character/BlasterCharacter.h"

void ABlasterPlayerController::BeginPlay()
{
    Super::BeginPlay();

    BlasterHUD = Cast<ABlasterHUD>(GetHUD());
}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn))
    {
        SetHUDHealth(BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());
    }
}

void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
    BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

    bool bIsHUDValid = BlasterHUD &&
        BlasterHUD->CharacterOverlay &&
        BlasterHUD->CharacterOverlay->HealthBar &&
        BlasterHUD->CharacterOverlay->HealthText;

    if (bIsHUDValid)
    {
        BlasterHUD->CharacterOverlay->HealthBar->SetPercent(Health / MaxHealth);

        FString HealthString = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
        BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthString));
    }
}

void ABlasterPlayerController::SetHUDScore(float NewScore)
{
    BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

    bool bIsHUDValid = BlasterHUD &&
        BlasterHUD->CharacterOverlay &&
        BlasterHUD->CharacterOverlay->ScoreAmount;

    if (bIsHUDValid)
    {
        FString ScoreString = FString::Printf(TEXT("%d"), FMath::FloorToInt(NewScore));
        BlasterHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreString));
    }
}

void  ABlasterPlayerController::SetHUDDefeat(int32 NewDefeat)
{
    BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

    bool bIsHUDValid = BlasterHUD &&
        BlasterHUD->CharacterOverlay &&
        BlasterHUD->CharacterOverlay->DefeatAmount;

    if (bIsHUDValid)
    {
        FString DefeatString = FString::Printf(TEXT("%d"), NewDefeat);
        BlasterHUD->CharacterOverlay->DefeatAmount->SetText(FText::FromString(DefeatString));
    }
}