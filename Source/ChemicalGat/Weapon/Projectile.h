// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UBoxComponent;
class UProjectileMovementComponent;
class UNiagaraSystem;
class UNiagaraComponent;
class USoundCue;

UCLASS()
class CHEMICALGAT_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** Called when this actor is explicitly being destroyed during gameplay or in the editor, 
	 * NOT called during level streaming or gameplay ending */
	virtual void Destroyed() override;

private:
	UPROPERTY(VisibleAnywhere)	
	UBoxComponent* HitBox;

	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(EditAnywhere, Category = VFX)
	UNiagaraSystem* BulletTrail;

	UNiagaraComponent* BulletTrailComponent;

	UPROPERTY(EditAnywhere, Category = VFX)
	UNiagaraSystem* ImpactParticle;

	UPROPERTY(EditAnywhere, Category = VFX)
	USoundCue* ImpactSound;
	
	
};
