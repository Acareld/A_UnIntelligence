// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "InspectorGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class A_UNINTELLIGENCE_API AInspectorGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	void BeginPlay();
	void RespawnPlayer(AController* Controller);
	void Countdown();
	void PauseTimer();
	void ResumeTimer();
	void NonHazardFound();
	
	UFUNCTION(BlueprintCallable)
	int32 CalculateRating();

	UFUNCTION(BlueprintImplementableEvent)
	void OnLevelEnded();

	virtual void PostLogin(APlayerController* NewPlayer) override;

	FTimerHandle TimerHandle;

	UPROPERTY(BlueprintReadOnly)
	bool bIsGameEnded = false;

protected:

	UPROPERTY(BlueprintReadOnly)
	int32 Seconds = 0;
	UPROPERTY(BlueprintReadOnly)
	int32 Minutes = 0;

	// Logging vars

	UPROPERTY(BlueprintReadOnly)
	int32 HazardsFound = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 NonHazardsFound = 0;

private:
	int32 SavedCamIndex;

	// Settings
	int32 MaxNumHazards = 7;
	int32 MaxNumNonHazards = 1;

	int32 MaxMinutes = 5;
	int32 MaxSeconds = 15;
};

