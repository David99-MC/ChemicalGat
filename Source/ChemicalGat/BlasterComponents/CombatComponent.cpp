// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "ChemicalGat/Character/BlasterCharacter.h"
#include "ChemicalGat/Weapon/Weapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

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
	// BlasterCharacter->bUseControllerRotationPitch = true;
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && BlasterCharacter)
	{
		BlasterCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
		BlasterCharacter->bUseControllerRotationYaw = true;
		// BlasterCharacter->bUseControllerRotationPitch = true;
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

