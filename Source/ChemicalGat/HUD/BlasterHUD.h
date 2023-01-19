// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

class UTexture2D;

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:
	UTexture2D* CenterCrosshair;
	UTexture2D* TopCrosshair;
	UTexture2D* BottomCrosshair;
	UTexture2D* LeftCrosshair;
	UTexture2D* RightCrosshair;
};

/**
 * 
 */
UCLASS()
class CHEMICALGAT_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()
	
protected:
	virtual void DrawHUD() override;

private:
	FHUDPackage HUDPackage;

public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& PackageToSet) { HUDPackage = PackageToSet; } 
};
