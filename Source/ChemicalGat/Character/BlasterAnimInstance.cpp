// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include "BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

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
    
    Speed = BlasterCharacter->GetVelocity().Size2D();
    bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();
    bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0 ? true : false;

    bEquippedWeapon = BlasterCharacter->GetIsWeaponEquipped();

    bIsCrouched = BlasterCharacter->bIsCrouched;
    bIsAiming = BlasterCharacter->GetIsAiming();
    
    AOYaw = BlasterCharacter->GetAOYaw();
    AOPitch = BlasterCharacter->GetAOPitch();  

    if (Speed > 3.f && bIsAccelerating)
        bShouldMove = true;
    else
        bShouldMove = false;

    SetYawOffset(DeltaTime);
    SetLean(DeltaTime);
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