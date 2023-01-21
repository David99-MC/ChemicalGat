// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class USphereComponent;
class UWidgetComponent;
class UAnimationAsset;
class AProjectile;
class UTexture2D;

UENUM(BlueprintType) // Enable this enum class in the blueprint
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class CHEMICALGAT_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();

	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void ShowPickupWidget(bool bShowWidget);
	
	void SetWeaponState(EWeaponState State);
	
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() { return WeaponMesh; }
	
	virtual void Fire(const FVector& HitTarget);

private:
	UFUNCTION()
	void OnRep_WeaponState();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnAreaSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnAreaSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
public:
	UPROPERTY(EditAnywhere, Category = Crosshair)
	UTexture2D* CenterCrosshair;

	UPROPERTY(EditAnywhere, Category = Crosshair)
	UTexture2D* TopCrosshair;

	UPROPERTY(EditAnywhere, Category = Crosshair)
	UTexture2D* BottomCrosshair;

	UPROPERTY(EditAnywhere, Category = Crosshair)
	UTexture2D* LeftCrosshair;

	UPROPERTY(EditAnywhere, Category = Crosshair)
	UTexture2D* RightCrosshair;

private:
	UPROPERTY(VisibleAnywhere, Category = WeaponProperties)
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = WeaponProperties)
	USphereComponent* AreaSphere;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_WeaponState, Category = WeaponProperties)
	EWeaponState WeaponState;

	UPROPERTY(VisibleAnywhere, Category = WeaponProperties)
	UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere, Category = WeaponProperties)
	UAnimationAsset* FireAnimation;	

protected:
	UPROPERTY(EditAnywhere, Category = WeaponProperties)
	TSubclassOf<AProjectile> ProjectileClass;

};
