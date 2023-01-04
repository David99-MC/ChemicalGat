// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class ABlasterCharacter;
class AWeapon;
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

public:
	void EquipWeapon(AWeapon* WeaponToEquip);

private:	
	ABlasterCharacter* BlasterCharacter;

	UFUNCTION()
	void OnRep_EquippedWeapon();

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bIsAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;
	
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;
};
