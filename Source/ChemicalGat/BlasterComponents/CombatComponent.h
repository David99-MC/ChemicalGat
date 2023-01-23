// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ChemicalGat/HUD/BlasterHUD.h"

#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80000.f

class ABlasterCharacter;
class AWeapon;
class ABlasterPlayerController;
class ABlasterHUD;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CHEMICALGAT_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	// Setting ABlasterCharacter a friend class so it will have access to all of this class' properties 
	// including from private sections
	friend class ABlasterCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	void SetAiming(bool bAiming); 
	
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bAiming);

	void FireButtonPressed(bool bPressed);
	
	// Called by the client BUT runs on server 
	UFUNCTION(Server, reliable)
	void ServerFireButtonPressed(const FVector_NetQuantize& TraceHitTarget);

	// Called by the server and runs on ALL machines
	UFUNCTION(NetMulticast, reliable)
	void MulticastFireButtonPressed(const FVector_NetQuantize& TraceHitTarget);

	void TraceLineUnderCrosshair(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

public:
	void EquipWeapon(AWeapon* WeaponToEquip);

private:	
	ABlasterCharacter* BlasterCharacter;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bIsAiming;

	UPROPERTY(EditAnywhere, Category = Combat)
	float BaseWalkSpeed;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	float AimWalkSpeed;

	bool bIsFiring;

	ABlasterPlayerController* BlasterController;

	FVector HitTarget;

	ABlasterHUD* BlasterHUD;

	UPROPERTY(EditAnywhere, Category = "Combat | Aiming")
	float CrosshairShrinkFactor = 30.f;

	UPROPERTY(EditAnywhere, Category = "Combat | Aiming")
	float DefaultCrosshairSpread = 0.55f;

	UPROPERTY(EditAnywhere, Category = "Combat | Aiming")
	float CrosshairJumpFactorMax;

	float CrosshairJumpFactor = 0.f;

	float CrosshairAimFactor;

	float CrosshairShootingFactor;

	FHUDPackage HUDPackage;

	/**
	 *  @param DefaultFOV default character follow camera's field of view
	 *  @param CurrentFOV determine how much the follow camera's fov should interp to 
	 *  NOTE: The closer to 0, the more zoomed it becomes
	*/
	float DefaultFOV;
	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = "Combat | Aiming")
	float UnZoomInterpSpeed = 20.f;

private:
	UFUNCTION()
	void OnRep_EquippedWeapon();

	void InterpFOV(float DeltaTime);
};
