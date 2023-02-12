// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "ChemicalGat/Weapon/Weapon.h"
#include "ChemicalGat/BlasterComponents/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Animation/AnimInstance.h"
#include "ChemicalGat/ChemicalGat.h"
#include "ChemicalGat/PlayerController/BlasterPlayerController.h"
#include "ChemicalGat/GameMode/BlasterGameMode.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Don't rotate the character when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 800.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->GravityScale = 3.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 450.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Overhead Widget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat Component"));
	Combat->SetIsReplicated(true);

	// Enabling the Character to crouch. Can also tick the box in the Character blueprint
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	TurnInPlace = ETurnInPlace::ETIP_NotTurning;

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("Dissolve Timeline Component"));
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, Health);
}

// Called when the game starts or when spawned
void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultBlasterMappingContext, 0);
		}
	}
	UpdateHUDHealth();
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
	}
}
// Called every frame
void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		SetAimOffsets(DeltaTime);
	}
	else // On simulated proxy
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication >= 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAOPitch();
	}

	HideMeshWhenCameraIsClose();
}

// Called to bind functionality to input
void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::Move);
		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::Look);
		// Equipping
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::Equip);
		// Aiming
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::Aim);
		// Shooting
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::Shoot);
		// Crouching
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::CrouchButtonPressed);
	}
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->BlasterCharacter = this;
	}
}

void ABlasterCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller)
	{
		// Find out which way is forward by getting the Controller's YAW rotation
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// Get the forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// Get the right vector
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// Add movement
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ABlasterCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (Controller)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ABlasterCharacter::Equip(const FInputActionValue& Value)
{
	// input is a bool
	bool bCanEquip = Value.Get<bool>();
	if (bCanEquip && Combat)
	{
		if (HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else // function being called from the client instead
		{
			ServerEquipButtonPressed();
		}
	}
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

void ABlasterCharacter::Jump()
{
	if (bIsCrouched) // Uncrouch when jumping while crouching
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void ABlasterCharacter::Aim(const FInputActionValue& Value)
{
	if (Combat)
		Combat->SetAiming(Value.Get<bool>());	
}

void ABlasterCharacter::Shoot(const FInputActionValue& Value)
{
	if (Combat)
		Combat->FireButtonPressed(Value.Get<bool>());
}

void ABlasterCharacter::CrouchButtonPressed(const FInputActionValue& Value)
{
	// Calling the Crouch/UnCrouch functions from the Character parent class
	if (bIsCrouched)
		UnCrouch();
	else
		Crouch();
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* CurrentWeapon)
{
	// OverlappingWeapon can be considered as *LastWeapon before getting new value from CurrentWeapon
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(false);
		}
	}
	OverlappingWeapon = CurrentWeapon;
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}

	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
}

bool ABlasterCharacter::GetIsAiming() const
{
	return (Combat && Combat->bIsAiming); 
}

void ABlasterCharacter::SetAimOffsets(float DeltaTime)
{
	if (GetEquippedWeapon() == nullptr)
	{
		// Update StartingBaseAimRotation even when unequipped
		StartingBaseAimRotation = FRotator(0, GetBaseAimRotation().Yaw, 0); 
		return;
	}
	float Speed = GetVelocity().Size2D();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed < 0.01f && !bIsInAir) // standing still and not jumping
	{
		bShouldRotateRootBone = true;
		FRotator CurrentBaseAimRotation = FRotator(0, GetBaseAimRotation().Yaw, 0);
		FRotator DeltaBaseAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentBaseAimRotation, StartingBaseAimRotation);
		AOYaw = DeltaBaseAimRotation.Yaw;
		if (TurnInPlace == ETurnInPlace::ETIP_NotTurning)
		{
			InterpAOYaw = AOYaw;
		}
		bUseControllerRotationYaw = true;
		SetTurnInPlace(DeltaTime);
	}

	if (Speed > 0.f || bIsInAir) // running or jumping
	{
		bShouldRotateRootBone = false;
		bUseControllerRotationYaw = true;
		StartingBaseAimRotation = FRotator(0, GetBaseAimRotation().Yaw, 0);
		AOYaw = 0;
		TurnInPlace = ETurnInPlace::ETIP_NotTurning;
	}

	CalculateAOPitch();
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();

	SimulatedProxiesTurn();
	TimeSinceLastMovementReplication = 0;
}
 
void ABlasterCharacter::SimulatedProxiesTurn()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) 
		return;

	bShouldRotateRootBone = false;
	float Speed = GetVelocity().Size2D();
	if (Speed > 0)
	{
		TurnInPlace = ETurnInPlace::ETIP_NotTurning;
		return;
	}
	
	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation(); 
	float ProxyDeltaRotationYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;
	UE_LOG(LogTemp, Warning, TEXT("Proxy Yaw: %f"), ProxyDeltaRotationYaw);
	if (FMath::Abs(ProxyDeltaRotationYaw) > TurnThreshold)
	{
		if (ProxyDeltaRotationYaw > TurnThreshold)
		{
			TurnInPlace = ETurnInPlace::ETIP_Right;
		}
		else if (ProxyDeltaRotationYaw < -TurnThreshold)
		{
			TurnInPlace = ETurnInPlace::ETIP_Left;
		}
		else
		{
			TurnInPlace = ETurnInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurnInPlace = ETurnInPlace::ETIP_NotTurning;

}

void ABlasterCharacter::CalculateAOPitch()
{
	AOPitch = GetBaseAimRotation().Pitch;
	if (AOPitch > 90.f && !IsLocallyControlled())
	{
		// Map AOPitch from [270, 360) to [-90, 0), taking the result when decompressing over the Internet
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AOPitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AOPitch);
	}
}

void ABlasterCharacter::SetTurnInPlace(float DeltaTime)
{
	if (AOYaw > 90.f)
	{
		TurnInPlace = ETurnInPlace::ETIP_Right;
	}
	else if (AOYaw < -90.f)
	{
		TurnInPlace = ETurnInPlace::ETIP_Left;
	}

	/** Turn in place code, interpolating the AOYaw down to Zero which will rotate the root bone back to facing forward */
	if (TurnInPlace != ETurnInPlace::ETIP_NotTurning)
	{
		InterpAOYaw = FMath::FInterpTo(InterpAOYaw, 0.f, DeltaTime, 5.f);
		AOYaw = InterpAOYaw;
		if (FMath::Abs(AOYaw) < 15.f) // Using Abs() for both directions
		{
			TurnInPlace = ETurnInPlace::ETIP_NotTurning;
			StartingBaseAimRotation = FRotator(0, GetBaseAimRotation().Yaw, 0);
		}
	}
}

void ABlasterCharacter::HideMeshWhenCameraIsClose()
{
	if (!IsLocallyControlled()) 
		return;

	if (FollowCamera && (FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		// if (GetEquippedWeapon())
		// {
		// 	GetEquippedWeapon()->GetWeaponMesh()->bOwnerNoSee = true;
		// }
	}
	else
	{
		GetMesh()->SetVisibility(true);
	}
}

AWeapon* ABlasterCharacter::GetEquippedWeapon() const
{
	if (Combat == nullptr) 
		return nullptr;
	return Combat->EquippedWeapon;
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if (!Combat) 
		return FVector();
	return Combat->HitTarget;
}

void ABlasterCharacter::PlayRifleMontage(bool bIsAiming)
{
	if (GetEquippedWeapon() == nullptr)
		return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && RifleMontage)
	{
		AnimInstance->Montage_Play(RifleMontage);
		FName SectionName = bIsAiming ? FName("HipFire") : FName("AimFire");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::OnHealthUpdate()
{
	// if (HasAuthority()) //Server-specific functionality

	// if (IsLocallyControlled()) //Client-specific functionality

	// Functions that happen on both server and client
	UpdateHUDHealth();

	if (bIsEliminated)
		return;
		
	PlayHitReactMontage();
}

/** change the player's current Health value on the server */
void ABlasterCharacter::ReceiveDamage(AActor* DamagedActor, float DamageAmount, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - DamageAmount, 0.f, MaxHealth);
	OnHealthUpdate();

	if (Health <= 0.f)
	{
		BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
		if (ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(GetWorld()->GetAuthGameMode()))
		{
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorController);
			BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);
		}
	}
}
  
/** Make sure the client react when their Health change as well */
void ABlasterCharacter::OnRep_Health() 
{
	OnHealthUpdate();
}

void ABlasterCharacter::UpdateHUDHealth()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
	if (GetEquippedWeapon() == nullptr) 
		return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName = FName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

/** This will only get called on the server where the game mode exists */
void ABlasterCharacter::PlayerElim()
{
	if (GetEquippedWeapon() != nullptr)
	{
		GetEquippedWeapon()->Drop();
	}

	MulticastPlayerElim();
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&ABlasterCharacter::ElimTimerFinished,
		ElimDelay
	);
}

/** Call from all machines */
void ABlasterCharacter::MulticastPlayerElim_Implementation()
{
	bIsEliminated = true;

	// Start dissolve effect
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);

		// After assigning the new dissolve material, set its scalar value before starting to dissolve
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 150.f);
	}
	StartDissolve();

	// Disable CharacterMovement
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if (BlasterPlayerController)
	{
		DisableInput(BlasterPlayerController);
	}

	// Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Spawn ElimBot at 200 units above the character
	FVector SpawnLocation = GetActorLocation();
	SpawnLocation.Z += 200.f;
	if (ElimBot)
	{
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ElimBot,
			SpawnLocation,
			GetActorRotation()
		);
	}
	if (ElimBotSoundCue)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ElimBotSoundCue, GetActorLocation(), 0.7f);
	}
}

void ABlasterCharacter::ElimTimerFinished()
{
	if (ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(GetWorld()->GetAuthGameMode()))
	{
		BlasterGameMode->RequestRespawn(this, Controller);
	}
}

void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();

	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}
}

void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void ABlasterCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ABlasterCharacter::UpdateDissolveMaterial);
	
	// Set up the timeline to use the DissolveCurve and associate it with the track which has a callback bound to it
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void ABlasterCharacter::StopAnimation()
{
	GetMesh()->bPauseAnims = true;
}