// Fill out your copyright notice in the Description page of Project Settings.


#include "InspectorGameModeBase.h"
#include "CharacterController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/NavigationObjectBase.h"
#include "Kismet/GameplayStatics.h"

void AInspectorGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	Minutes = MaxMinutes;
	Seconds = MaxSeconds;
	bIsGameEnded = false;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &AInspectorGameModeBase::Countdown, 1.f, true, 0.0f);
}

void AInspectorGameModeBase::RespawnPlayer(AController* Controller)
{
	if (!Controller) return;
	APawn* CurrentPawn = Controller->GetPawn();

	if (!CurrentPawn) return;

	ACharacterController* PC = Cast<ACharacterController>(CurrentPawn);
	FTransform Spawn;

	if (PC)
	{
		Spawn = PC->GetRespawnTransform();
		SavedCamIndex = PC->GetCurrentCamIndex();
	}

	if (!Spawn.IsValid())
	{
		AActor* Start = UGameplayStatics::GetActorOfClass(this, APlayerStart::StaticClass());
		Spawn = Start ? Start->GetActorTransform() : FTransform();
	}

	APawn* OldPawn = Controller->GetPawn();
	if (OldPawn)
	{
		OldPawn->Destroy();
	}

	UWorld* World = GetWorld();
	if (!World) return;

	UClass* PawnClass = GetDefaultPawnClassForController(Controller);
	if (!PawnClass) return;

	APawn* NewPawn = World->SpawnActor<APawn>(PawnClass, Spawn);
	if (!NewPawn) return;

	Controller->Possess(NewPawn);

	if (ACharacterController* NewPC = Cast<ACharacterController>(NewPawn))
	{
		NewPC->SetCurrentCamIndex(SavedCamIndex);
		NewPC->SetCorrectViewTarget();
		//NewPC->HandleRespawn();
	}

	HazardsFound++;

	if (HazardsFound >= MaxNumHazards)
	{
		OnLevelEnded();
		bIsGameEnded = true;
		return;
	}

	ResumeTimer();
}

void AInspectorGameModeBase::NonHazardFound()
{
	NonHazardsFound++;
}

int32 AInspectorGameModeBase::CalculateRating()
{
	float NonHazardRatio = MaxNumNonHazards > 0 ? (float)NonHazardsFound / MaxNumNonHazards : 0.0f;
	float HazardRatio = MaxNumHazards > 0 ? (float)HazardsFound / MaxNumHazards : 0.0f;

	int32 TotalTime = MaxMinutes * 60 + MaxSeconds;
	int32 RemainingTime = Minutes * 60 + Seconds;

	float TimeRatio = TotalTime > 0 ? (float)RemainingTime / TotalTime : 0.0f;

	float Score =
		(TimeRatio * 0.3f +
			HazardRatio * 0.5f +
			NonHazardRatio * 0.2f) * 4000;

	return Score;
}

void AInspectorGameModeBase::Countdown()
{
	if (Seconds != 0)
	{
		--Seconds;
	}
	else
	{
		if (Minutes == 0)
		{
			OnLevelEnded();
			bIsGameEnded = true;
		}
		else
		{
			--Minutes;
			Seconds = 59;
		}
	}
}

void AInspectorGameModeBase::PauseTimer()
{
	GetWorldTimerManager().PauseTimer(TimerHandle);
}

void AInspectorGameModeBase::ResumeTimer()
{
	GetWorldTimerManager().UnPauseTimer(TimerHandle);
}

void AInspectorGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!NewPlayer) return;

	APawn* Pawn = NewPlayer->GetPawn();
	if (!Pawn) return;

	if (ACharacterController* Character = Cast<ACharacterController>(Pawn))
	{
		//Character->HandleRespawn(); 
	}
}