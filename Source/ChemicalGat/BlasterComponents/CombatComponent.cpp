#include "CombatComponent.h"
#include "ChemicalGat/Character/BlasterCharacter.h"
#include "ChemicalGat/Weapon/Weapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "ChemicalGat/PlayerController/BlasterPlayerController.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	if (BlasterCharacter)
	{
		BlasterCharacter->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		if (BlasterCharacter->GetFollowCamera())
		{
			DefaultFOV = BlasterCharacter->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bIsAiming);
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (BlasterCharacter && BlasterCharacter->IsLocallyControlled())
	{
		SetHUDCrosshairs(DeltaTime);

		FHitResult HitResult;
		TraceLineUnderCrosshair(HitResult);
		HitTarget = HitResult.ImpactPoint;

		InterpFOV(DeltaTime);

	}
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
			
			if (EquippedWeapon) // Only draw the crosshair when holding a weapon
			{
				HUDPackage.TopCrosshair = EquippedWeapon->TopCrosshair;
				HUDPackage.BottomCrosshair = EquippedWeapon->BottomCrosshair;
				HUDPackage.LeftCrosshair = EquippedWeapon->LeftCrosshair;
				HUDPackage.RightCrosshair = EquippedWeapon->RightCrosshair;
				HUDPackage.CenterCrosshair = EquippedWeapon->CenterCrosshair;

				// Calculating the Spread based on running or jumping
				// Map the character's velocity to the velocity multiplier range, i.e. Mapping [0, 600] -> [0, 1]
				// Ex: Velocity.Size2D() is [0, 1] based on character's walk speed 
				FVector2D WalkSpeedRange(0, BlasterCharacter->GetCharacterMovement()->MaxWalkSpeed);
				FVector2D VelocityMultiplierRange(0, 1.f);
				FVector Velocity = BlasterCharacter->GetVelocity();

				float CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size2D());
				
				if (BlasterCharacter->GetCharacterMovement()->IsFalling())
				{
					CrosshairJumpFactor = FMath::FInterpTo(CrosshairJumpFactor, CrosshairJumpFactorMax, DeltaTime, 2.25f);
				}
				else
				{
					CrosshairJumpFactor = FMath::FInterpTo(CrosshairJumpFactor, 0.f, DeltaTime, CrosshairShrinkFactor);
				}

				if (bIsAiming)
				{
					CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.5f, DeltaTime, CrosshairShrinkFactor);
				}
				else
				{
					CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0, DeltaTime, CrosshairShrinkFactor);
				}

				CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0, DeltaTime, 2.f);
	
				HUDPackage.CrosshairSpread =
					DefaultCrosshairSpread + 
					CrosshairVelocityFactor + 
					CrosshairJumpFactor -
					CrosshairAimFactor +
					CrosshairShootingFactor;
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

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (!BlasterCharacter->GetFollowCamera() || !EquippedWeapon) 
		return;
	
	if (bIsAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomedInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, UnZoomInterpSpeed);
	}
	BlasterCharacter->GetFollowCamera()->SetFieldOfView(CurrentFOV);
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
	if (bIsFiring && EquippedWeapon)
	{
		Fire();
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

void UCombatComponent::Fire()
{
	if (bCanFire)
	{
		ServerFireButtonPressed(HitTarget);
		bCanFire = false;
		if (EquippedWeapon)
		{
			CrosshairShootingFactor = EquippedWeapon->GetCrosshairShootingFactor();
		}
		StartFireTimer();
	}
}

void UCombatComponent::StartFireTimer()
{
	if (BlasterCharacter && EquippedWeapon)
	{
		BlasterCharacter->GetWorldTimerManager().SetTimer(FireTimer, this, &UCombatComponent::FireTimerFinished, EquippedWeapon->GetFireDelay());
	}
}

void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr) 
		return;

	bCanFire = true;
	if (bIsFiring && EquippedWeapon->IsAutomatic()) // keep firing if the button is still being press
	{
		Fire();
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
		if (BlasterCharacter)
		{
			float DistanceToCharacter = (BlasterCharacter->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 50.f); 
			// DrawDebugSphere(GetWorld(), Start, 16.f, 12, FColor::Red, false);
		}

		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		// Perform Line tracing to the middle of the screen
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);

		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairInterface>())
		{
			HUDPackage.CrosshairColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairColor = FLinearColor::White;
		}

		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End; 
		}
	}
}