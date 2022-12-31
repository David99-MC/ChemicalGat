// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include "BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

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
    
    FVector Velocity = BlasterCharacter->GetVelocity();
    Velocity.Z = 0;
    Speed = Velocity.Size();

    bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();
    bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0 ? true : false;

    if (Speed > 3.f && bIsAccelerating)
        bShouldMove = true;
    else
        bShouldMove = false;

    bEquippedWeapon = BlasterCharacter->IsWeaponEquipped();
    bIsCrouched = BlasterCharacter->bIsCrouched;
}