// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"
#include "CharacterOverlay.h"
#include "GameFramework/PlayerController.h"

void ABlasterHUD::BeginPlay()
{
    Super::BeginPlay();
    AddCharacterOverlay();
}

void ABlasterHUD::AddCharacterOverlay()
{
    APlayerController* PlayerController = GetOwningPlayerController();
    if (PlayerController && CharacterOverlayClass)
    {
        CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
        CharacterOverlay->AddToViewport();
    }
}

void ABlasterHUD::DrawHUD()
{
    Super::DrawHUD();

    FVector2D ViewportSize;
    if (GEngine)
    {
        GEngine->GameViewport->GetViewportSize(ViewportSize);
        const FVector2D ViewportCenter2D(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

        // Only draw the crosshair if all of the crosshair components are valid
        if (HUDPackage.BottomCrosshair && HUDPackage.CenterCrosshair && HUDPackage.LeftCrosshair && HUDPackage.RightCrosshair && HUDPackage.TopCrosshair)
        {
            float CrosshairScaled = HUDPackage.CrosshairSpread * CrosshairSpreadMax;

            // Change Y for Top and Bottom, X for Left and Right
            DrawCrosshair(HUDPackage.CenterCrosshair, ViewportCenter2D, FVector2D::ZeroVector, HUDPackage.CrosshairColor);
            DrawCrosshair(HUDPackage.TopCrosshair, ViewportCenter2D, FVector2D(0, -CrosshairScaled), HUDPackage.CrosshairColor);
            DrawCrosshair(HUDPackage.BottomCrosshair, ViewportCenter2D, FVector2D(0, CrosshairScaled), HUDPackage.CrosshairColor);
            DrawCrosshair(HUDPackage.LeftCrosshair, ViewportCenter2D, FVector2D(-CrosshairScaled, 0), HUDPackage.CrosshairColor);
            DrawCrosshair(HUDPackage.RightCrosshair, ViewportCenter2D, FVector2D(CrosshairScaled, 0), HUDPackage.CrosshairColor);
        }
    }
}

/**
 * To draw the crosshair components exactly at the center of the viewport,
 * draw the texture with its upperleft corner at the viewport center (move them to the LEFT and UP by half of the texture size) 
*/
void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, const FVector2D& ViewportCenter, const FVector2D& SpreadFactor, FLinearColor CrosshairColor)
{
    float TextureX = ViewportCenter.X - (Texture->GetSizeX() / 2) + SpreadFactor.X;
    float TextureY = ViewportCenter.Y - (Texture->GetSizeY() / 2) + SpreadFactor.Y;

    DrawTexture(
        Texture,
        TextureX,
        TextureY,
        Texture->GetSizeX(),
        Texture->GetSizeY(),
        0.f,
        0.f,
        1.f,
        1.f,
        CrosshairColor
    );
}