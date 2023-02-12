// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "ChemicalGat/BlasterTypes/TurningInPlace.h"
#include "ChemicalGat/Interfaces/InteractWithCrosshairInterface.h"
#include "Components/TimelineComponent.h"

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
class ABlasterPlayerController;
class UParticleSystem;
class UParticleSystemComponent;
class USoundCue;

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
	FORCEINLINE bool GetIsEliminated() const { return bIsEliminated; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	AWeapon* GetEquippedWeapon() const;
	bool GetIsAiming() const;
	void SetOverlappingWeapon(AWeapon* Weapon);
	FVector GetHitTarget() const;

	void PlayRifleMontage(bool bIsAiming);
	void PlayHitReactMontage();
	void PlayerElim();

	void StopAnimation();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayerElim();

	void ElimTimerFinished();

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

	UFUNCTION()
	virtual void ReceiveDamage(AActor* DamagedActor, float DamageAmount, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);

	virtual void Destroyed() override;

private:
	// A Remote Procedure Call (RPC) to allow the client to also pick up the weapon 
	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UFUNCTION()
	void OnRep_Health();

	void SetAimOffsets(float DeltaTime);
	void SetTurnInPlace(float DeltaTime);
	void HideMeshWhenCameraIsClose();
	void SimulatedProxiesTurn();
	void CalculateAOPitch();
	void UpdateHUDHealth();
	
	void OnHealthUpdate();

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);

	void StartDissolve();

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

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* RifleMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;

	FVector HitTarget;

	UPROPERTY(EditAnywhere, Category = Combat)
	float CameraThreshold = 200.f;

	/** 
	 * Implementing Aimoffsets
	 * 
	 * @param AOYaw set Yaw Value of the Blendspace
	 * @param InterpAOYaw reset the AOYaw to 0 for turning in place  
	 * @param AOPitch set Lean Value of the Blendspace
	*/
	float AOYaw;
	float InterpAOYaw;
	float AOPitch;

	FRotator StartingBaseAimRotation;

	ETurnInPlace TurnInPlace;

	UPROPERTY(EditAnywhere, Category = WeaponRotationCorrection)
	float RightHandRotationRollOffset;

	/**
	 * Smoothing Proxies' rotation
	*/
	bool bShouldRotateRootBone;
	float TurnThreshold = 1.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float TimeSinceLastMovementReplication;

	/**
	 * Player Health and Stats
	*/
	UPROPERTY(EditAnywhere, Category = Stats)
	float MaxHealth = 100.f;
	
	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = Stats)
	float Health = 100.f;

	// Getting the Controller for HUD related functionalities
	ABlasterPlayerController* BlasterPlayerController;

	/**
	 *  Elimination and Respawn
	*/
	bool bIsEliminated;

	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;

	/**
	 *  Dissolve effect
	 * 
	 * @param DissolveTrack the track with key frames used for the timeline
	 * @param DissolveTimeline Timeline component which is bound by a callback that's called every frame to update the value on the curve
	 * @param DissolveCurve responsible for smoothing the ScalarParameter Dissolve value
	*/
	UPROPERTY(VisibleAnywhere, Category = Elim)
	UTimelineComponent* DissolveTimeline;

	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere, Category = Elim)
	UCurveFloat* DissolveCurve;

	// Material instance that can be changed at run time
	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	// Material used on the blueprint, used with the dynamic material
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance;

	UPROPERTY(EditAnywhere, Category = Elim)
	UParticleSystem* ElimBot;

	UPROPERTY(VisibleAnywhere, Category = Elim)
	UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditAnywhere, Category = Elim)
	USoundCue* ElimBotSoundCue;

public:

};
