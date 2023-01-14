// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
    Super::Fire(HitTarget);

    APawn* PawnInstigator = Cast<APawn>(GetOwner());
    const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleSocket"));
    if (MuzzleFlashSocket && PawnInstigator)
    {
        FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
        FVector SpawnLocation = SocketTransform.GetLocation();
        FVector TargetDirection = HitTarget - SpawnLocation; 
        FRotator ToTargetRotation = TargetDirection.Rotation(); // Rotate the Spawned projectile in the given direction
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = GetOwner();
        SpawnParams.Instigator = PawnInstigator;

        UWorld* World = GetWorld();
        if (World)
        {
            World->SpawnActor<AProjectile>(
                ProjectileClass,
                SpawnLocation,
                ToTargetRotation,
                SpawnParams
            );
        }
    }

}