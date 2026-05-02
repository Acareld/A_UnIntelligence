// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

UENUM(BlueprintType)
enum class EInteractionType : uint8
{
	None		UMETA(DisplayName = "None"),
	Pickup		UMETA(DisplayName = "Pickup"),
	Trap		UMETA(DisplayName = "Trap")
};

/**
 * 
 */
class A_UNINTELLIGENCE_API IInteractable
{
	GENERATED_BODY()

protected:
	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual EInteractionType GetInteractionType() const = 0;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(APawn* InstigatorPawn);

	//-------------
	// Not used
	virtual bool CanPickup(AActor* ByActor) { return false; };
	virtual void OnPickup(AActor* ByActor) {};

	virtual void OnDrop(AActor* ByActor) {};
	//-------------
};

