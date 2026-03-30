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
	virtual void PostLogin(APlayerController* NewPlayer) override;

	FTimerHandle TimerHandle;

	UPROPERTY(BlueprintReadOnly)
	int Seconds = 15;
	UPROPERTY(BlueprintReadOnly)
	int Minutes = 5;

	UPROPERTY(BlueprintReadOnly)
	int HazardsFound = 0;
private:
	int32 SavedCamIndex;
};

