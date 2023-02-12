// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.generated.h"


class ABlasterCharacter;
class ABlasterPlayerController;
/**
 * 
 */
UCLASS()
class CHEMICALGAT_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

	void AddToScore(float ScoreAmount);
	virtual void OnRep_Score() override;

	void AddToDefeat(int32 DefeateAmount);
	UFUNCTION()
	virtual void OnRep_Defeat();

private:
	bool IsControllerValid();
	void OnScoreUpdate();
	void OnDefeatUpdate();

private:
	UPROPERTY()
	ABlasterCharacter* BlasterCharacter;
	
	UPROPERTY()
	ABlasterPlayerController* BlasterController;

	UPROPERTY(ReplicatedUsing = OnRep_Defeat)
	int32 Defeat = 0.f;
};
