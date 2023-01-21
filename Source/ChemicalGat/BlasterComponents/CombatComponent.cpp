// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "ChemicalGat/Character/BlasterCharacter.h"
#include "ChemicalGat/Weapon/Weapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "ChemicalGat/PlayerController/BlasterPlayerController.h"
#include "ChemicalGat/HUD/BlasterHUD.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	if (BlasterCharacter)
		BlasterCharacter->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bIsAiming);
}

// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SetHUDCrosshairs(DeltaTime);
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (BlasterCharacter == nullptr || BlasterCharacter->Controller == nullptr) return;

	BlasterController = BlasterController == nullptr ? Cast<ABlasterPlayerController>(BlasterCharacter->Controller) : BlasterController;
	if (BlasterController)
	{
		BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(BlasterController->GetHUD()) : BlasterHUD;
		if (BlasterHUD)
		{
			FHUDPackage HUDPackage;
			if (EquippedWeapon) // Only draw the crosshair when holding a weapon
			{
				HUDPackage.TopCrosshair = EquippedWeapon->TopCrosshair;
				HUDPackage.BottomCrosshair = EquippedWeapon->BottomCrosshair;
				HUDPackage.LeftCrosshair = EquippedWeapon->LeftCrosshair;
				HUDPackage.RightCrosshair = EquippedWeapon->RightCrosshair;
				HUDPackage.CenterCrosshair = EquippedWeapon->CenterCrosshair;

				// Calculating the Spread based on running or jumping
				// The idea is to map the character's velocity to the velocity multiplier range, i.e. Mapping [0, 600] -> [0, 1]
				// Ex: Velocity.Size2D() is [0, 1] based on character's walk speed 
				FVector2D WalkSpeedRange(0, BaseWalkSpeed);
				FVector2D VelocityMultiplierRange(0, 1.f);
				FVector Velocity = BlasterCharacter->GetVelocity();

				float CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size2D());
				
				if (BlasterCharacter->GetCharacterMovement()->IsFalling())
				{
					JumpingFactor = FMath::FInterpTo(JumpingFactor, JumpingFactorMax, DeltaTime, 2.25f);
				}
				else
				{
					JumpingFactor = FMath::FInterpTo(JumpingFactor, 0.f, DeltaTime, 30.f);
				}

				HUDPackage.CrosshairSpread = CrosshairVelocityFactor + JumpingFactor;
			}
			else
			{
				HUDPackage.TopCrosshair = nullptr;
				HUDPackage.BottomCrosshair = nullptr;
				HUDPackage.LeftCrosshair = nullptr;
				HUDPackage.RightCrosshair = nullptr;
				HUDPackage.CenterCrosshair = nullptr;
				HUDPackage.CrosshairSpread = 0.f;
			}
			BlasterHUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (!BlasterCharacter || !WeaponToEquip) return;

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	// Equipping the weapon
	const USkeletalMeshSocket* HandSocket = BlasterCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, BlasterCharacter->GetMesh());
	}
	EquippedWeapon->SetOwner(BlasterCharacter);
	BlasterCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
	BlasterCharacter->bUseControllerRotationYaw = true;
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && BlasterCharacter)
	{
		BlasterCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
		BlasterCharacter->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::SetAiming(bool bAiming)
{
	bIsAiming = bAiming;
	ServerSetAiming(bIsAiming);
	BlasterCharacter->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
}

void UCombatComponent::ServerSetAiming_Implementation(bool bAiming)
{
	bIsAiming = bAiming;
	BlasterCharacter->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bIsFiring = bPressed;
	if (bIsFiring)
	{
		FHitResult HitResult;
		TraceLineUnderCrosshair(HitResult); // filling in the HitResult information
		ServerFireButtonPressed(HitResult.ImpactPoint);
	}
}

void UCombatComponent::ServerFireButtonPressed_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFireButtonPressed(TraceHitTarget);
}

void UCombatComponent::MulticastFireButtonPressed_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (BlasterCharacter && EquippedWeapon)
	{
		BlasterCharacter->PlayRifleMontage(bIsAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::TraceLineUnderCrosshair(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// Setting the cross hair at the middle of the view port
	FVector2D CrosshairLocation2D(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	
	FVector CrosshairWorldLocation;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation2D,
		CrosshairWorldLocation,
		CrosshairWorldDirection
	);

	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldLocation;
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		// Perform Line tracing to the middle of the screen
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);
		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End; 
		}
	}
	
}