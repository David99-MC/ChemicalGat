// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "ChemicalGat/BlasterTypes/TurningInPlace.h"
#include "BlasterCharacter.generated.h"

/** Forward Declarations */
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UWidgetComponent;
class AWeapon;
class UCombatComponent;

UCLASS()
class CHEMICALGAT_API ABlasterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// Register any variables to be replicated inside this function
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

protected:
	virtual void BeginPlay() override;
	/** Bind to movement input */
	void Move(const FInputActionValue& Value);
	/** Bind to looking input */
	void Look(const FInputActionValue& Value);
	/** Bind to Equipping input */
	void Equip(const FInputActionValue& Value);
	/** Bind to Aiming input */
	void Aim(const FInputActionValue& Value);
	/** Bind to Crouching input */
	void CrouchButtonPressed(const FInputActionValue& Value);
	
	virtual void Jump() override;

public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE float GetAOYaw() const { return AOYaw; }
	FORCEINLINE float GetAOPitch() const { return AOPitch; }
	FORCEINLINE ETurnInPlace GetTurnInPlace() const { return TurnInPlace; }
	AWeapon* GetEquippedWeapon() const;
	bool GetIsWeaponEquipped() const;
	bool GetIsAiming() const;

	void SetOverlappingWeapon(AWeapon* Weapon);
private: // Variables
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultBlasterMappingContext;

	/** InputAction variables to be bound in UE editor */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EquipAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AimAction;
	/** ---------------------------------------------- */

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = HUD, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* OverheadWidget;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;

	UPROPERTY(VisibleAnywhere)
	UCombatComponent* Combat;

	/** 
	 * @param AOYaw: used to set Yaw Value of the Blendspace
	 * @param InterpAOYaw: Used to reset the AOYaw to 0 for turning in place  
	 * @param AOPitch: Used to set Lean Value of the Blendspace
	*/
	float AOYaw;
	float InterpAOYaw;
	float AOPitch;

	FRotator StartingBaseAimRotation;

	ETurnInPlace TurnInPlace;

private: 
	// A Remote Procedure Call (RPC) to allow the client to also pick up the weapon 
	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	void SetAimOffsets(float DeltaTime);

	void SetTurnInPlace(float DeltaTime);

};
