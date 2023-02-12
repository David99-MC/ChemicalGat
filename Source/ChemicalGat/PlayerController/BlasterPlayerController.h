// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

class ABlasterHUD;
/**
 * 
 */
UCLASS()
class CHEMICALGAT_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

private:
	ABlasterHUD* BlasterHUD;

public:
	void SetHUDHealth(float Health, float MaxHealth); // to be called on BlasterCharacter class
	virtual void OnPossess(APawn* InPawn) override;
};
