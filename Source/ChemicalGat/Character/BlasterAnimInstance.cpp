
#include "BlasterAnimInstance.h"
#include "BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "ChemicalGat/Weapon/Weapon.h"

void UBlasterAnimInstance::NativeInitializeAnimation() 
{
    Super::NativeInitializeAnimation();
    BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaTime) 
{
    Super::NativeUpdateAnimation(DeltaTime);
    if (BlasterCharacter == nullptr)
        BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
    
    if (!BlasterCharacter) 
        return;

    UpdateLocomotion();
	UpdateWeapon();
	UpdateCombat();
    
    SetYawOffset(DeltaTime);
    SetLean(DeltaTime);

    if (bIsWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && BlasterCharacter->GetMesh())
        AttachLeftHandToWeapon();
}

void UBlasterAnimInstance::UpdateLocomotion()
{
    Speed = BlasterCharacter->GetVelocity().Size2D();
    bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();
    bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0 ? true : false;
    if (Speed > 3.f && bIsAccelerating)
        bShouldMove = true;
    else
        bShouldMove = false;
}

void UBlasterAnimInstance::UpdateCombat()
{
    bIsCrouched = BlasterCharacter->bIsCrouched;
    bIsAiming = BlasterCharacter->GetIsAiming();
    
    AOYaw = BlasterCharacter->GetAOYaw();
    AOPitch = BlasterCharacter->GetAOPitch();
}

void UBlasterAnimInstance::UpdateWeapon()
{
    bIsWeaponEquipped = BlasterCharacter->GetIsWeaponEquipped();
    EquippedWeapon = BlasterCharacter->GetEquippedWeapon();
}

/**
 * Transform the LeftHandSocket into bone space for one of the bones on the Character's mesh
 * We want the socket location on the weapon relative to the right hand
 * => Because the weapon should NOT be adjusted or moved relative to the right hand at runtime during the game
*/
void UBlasterAnimInstance::AttachLeftHandToWeapon()
{
    LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
    FVector OutLocation;
    FRotator OutRotation;
    BlasterCharacter->GetMesh()->TransformToBoneSpace(  FName("Hand_R"), 
                                                        LeftHandTransform.GetLocation(), // InLocation
                                                        FRotator::ZeroRotator,           // InRotation
                                                        OutLocation, 
                                                        OutRotation );
    // After calling the function, OutLocation and OutRotation will have the correct data of the LeftHandSocket on the weapon
    // transformed to Hand_R bone space, ready to be used to set the new location and rotation on the LeftHandSocket
    LeftHandTransform.SetLocation(OutLocation);
    LeftHandTransform.SetRotation(FQuat(OutRotation));
}

void UBlasterAnimInstance::SetYawOffset(float DeltaTime)
{
    // Calculating YawOffset for strafing
    FRotator AimRotation = BlasterCharacter->GetBaseAimRotation();
    FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());
    FRotator RotationTarget = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
    DeltaRotation = FMath::RInterpTo(DeltaRotation, RotationTarget, DeltaTime, 6.f);
    YawOffset = DeltaRotation.Yaw;
}

void UBlasterAnimInstance::SetLean(float DeltaTime)
{
    // Calculating Lean for leaning
    CharacterRotationLastFrame = CharacterRotation;
    CharacterRotation = BlasterCharacter->GetActorRotation();
    FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
    float Target = Delta.Yaw / DeltaTime;
    float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
    Lean = FMath::Clamp(Interp, -90, 90);
}