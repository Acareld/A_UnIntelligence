// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "GameplayTagContainer.h"
#include "PickupItem.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

UCLASS()
class A_UNINTELLIGENCE_API APickupItem : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickupItem();

	virtual EInteractionType GetInteractionType() const override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditInstanceOnly, Category="ItemType")
	FGameplayTag ItemTag;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Pickup")
	bool bIsHeld = false;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* InteractionVolume;

	FRotator InitialRotation;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual bool CanPickup(AActor* ByActor) override;
	virtual void OnPickup(AActor* ByActor) override;
	virtual void OnDrop(AActor* ByActor) override;
};
