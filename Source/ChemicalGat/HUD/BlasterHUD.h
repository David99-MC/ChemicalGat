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

public: // All these values to be set in the CombatComponent
	UTexture2D* CenterCrosshair;
	UTexture2D* TopCrosshair;
	UTexture2D* BottomCrosshair;
	UTexture2D* LeftCrosshair;
	UTexture2D* RightCrosshair;
	float CrosshairSpread; 
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

	// Scale the CrosshairSpread by this variable
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 15.f;

private:
	/**
	 * @param SpreadFactor	Represented by an FVector2D to determine whether it should spread in the X or Y direction 
	*/
	void DrawCrosshair(UTexture2D* Texture, const FVector2D& ViewportCenter, const FVector2D& SpreadFactor);

public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& PackageToSet) { HUDPackage = PackageToSet; } 
};
