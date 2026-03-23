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

		}
		else
		{
			--Minutes;
			Seconds = 59;
		}
	}
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