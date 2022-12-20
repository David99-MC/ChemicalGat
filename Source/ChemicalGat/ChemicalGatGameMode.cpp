// Copyright Epic Games, Inc. All Rights Reserved.

#include "ChemicalGatGameMode.h"
#include "ChemicalGatCharacter.h"
#include "UObject/ConstructorHelpers.h"

AChemicalGatGameMode::AChemicalGatGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
