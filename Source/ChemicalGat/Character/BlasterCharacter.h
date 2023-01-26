// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "ChemicalGat/BlasterTypes/TurningInPlace.h"
#include "ChemicalGat/Interfaces/InteractWithCrosshairInterface.h"

#include "BlasterCharacter.generated.h"

/** Forward Declarations */
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UWidgetComponent;
class AWeapon;
class UCombatComponent;
class UAnimMontage;

UCLASS()
class CHEMICALGAT_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairInterface
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// Register any variables to be replicated inside this function
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE float GetAOYaw() const { return AOYaw; }
	FORCEINLINE float GetAOPitch() const { return AOPitch; }
	FORCEINLINE ETurnInPlace GetTurnInPlace() const { return TurnInPlace; }
	FORCEINLINE float GetRightHandRotationRollOffset() const { return RightHandRotationRollOffset; }
	FORCEINLINE bool GetShouldRotateRootBone() const { return bShouldRotateRootBone; }
	
	AWeapon* GetEquippedWeapon() const;
	bool GetIsAiming() const;
	void SetOverlappingWeapon(AWeapon* Weapon);
	FVector GetHitTarget() const;
	void PlayRifleMontage(bool bIsAiming);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastHit();

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
	/** Bind to Shooting input */
	void Shoot(const FInputActionValue& Value);
	
	/** Bind to Crouching input */
	void CrouchButtonPressed(const FInputActionValue& Value);

	virtual void Jump() override;

	virtual void OnRep_ReplicatedMovement() override;

private:
	// A Remote Procedure Call (RPC) to allow the client to also pick up the weapon 
	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	void SetAimOffsets(float DeltaTime);
	void SetTurnInPlace(float DeltaTime);
	void HideMeshWhenCameraIsClose();
	void PlayHitReactMontage();
	void SimulatedProxiesTurn();
	void CalculateAOPitch();

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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ShootAction;
	/** ---------------------------------------------- */

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = HUD, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* OverheadWidget;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;

	UPROPERTY(VisibleAnywhere)
	UCombatComponent* Combat;

	/** 
	 * @param AOYaw used to set Yaw Value of the Blendspace
	 * @param InterpAOYaw used to reset the AOYaw to 0 for turning in place  
	 * @param AOPitch used to set Lean Value of the Blendspace
	*/
	float AOYaw;
	float InterpAOYaw;
	float AOPitch;

	FRotator StartingBaseAimRotation;

	ETurnInPlace TurnInPlace;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* RifleMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;

	FVector HitTarget;

	UPROPERTY(EditAnywhere, Category = Combat)
	float CameraThreshold = 200.f;

	UPROPERTY(EditAnywhere, Category = WeaponRotationCorrection)
	float RightHandRotationRollOffset;
	
	bool bShouldRotateRootBone;
	float TurnThreshold = 15.f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float TimeSinceLastMovementReplication;

public:

};
