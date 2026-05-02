// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "CameraRegionListener.h"
#include "CharacterController.generated.h"

class ACameraRegionCollider;

//----------------
// Not used
USTRUCT()
struct FActiveRegion
{
	GENERATED_BODY()

	UPROPERTY() TObjectPtr<ACameraRegionCollider> Region = nullptr;
	UPROPERTY() double EnterTime = 0.0;
};
//----------------

UCLASS()
class A_UNINTELLIGENCE_API ACharacterController : public ACharacter, public ICameraRegionListener
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACharacterController();

	virtual void OnCameraRegionEntered_Implementation(ACameraRegionCollider* Region) override;
	virtual void OnCameraRegionExited_Implementation(ACameraRegionCollider* Region) override;

protected:
	void Move(const FInputActionValue& Value);
	virtual void PawnClientRestart() override;
	void ApplyInputMapping();
	void Interact();
	AActor* GetBestInteractable() const;

	UPROPERTY(EditAnywhere, Category = "Input")
	TSoftObjectPtr<UInputMappingContext> InputMapping;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* CamAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* DropAction;

	FVector Forward = FVector(1.f, 0.f, 0.f);
	FVector Right = FVector(0.f, 1.f, 0.f);

	// ----------------------
	// Not used

	ACameraRegionCollider* ChooseBestRegion() const;
	void RecomputeAndApplyCamera();
	void Pickup(AActor* Item);
	void Drop();
	void DropPressed();
	void CamSwitch();

	UPROPERTY()
	TArray<FActiveRegion> ActiveRegions;

	UPROPERTY(EditAnywhere)
	UAnimationAsset* RespawnAnim;

	UPROPERTY(EditAnywhere)
	FTransform RespawnTransform;


	UPROPERTY(EditAnywhere, Category = "Region")
	AActor* DefaultViewTarget;

	UPROPERTY()
	TArray<TObjectPtr<class ACameraActor>> LevelCameras;

	UPROPERTY()
	AActor* HeldItem = nullptr;

	// ----------------------------
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void PlayAnimation(UAnimationAsset* Anim);
	void SetRotation(FRotator Rotation);
	FTransform GetRespawnTransform() const { return RespawnTransform; }

	// --------------------------
	// Not used
	void SetRespawnTransform(const FTransform& NewTransform);
	FGameplayTag GetHeldItemTag() const;
	void DeleteHeldItem();
	void HandleRespawn();
	int32 GetCurrentCamIndex() const { return CurrentCamIndex; };
	void SetCurrentCamIndex(int32 NewCamIndex) { CurrentCamIndex = NewCamIndex; };
	void SetCorrectViewTarget();
	void AutoCamSwitch();
	// --------------------------
	
	UPROPERTY()
	TSet<TObjectPtr<AActor>> NearbyInteractables;

	UPROPERTY()
	TObjectPtr<AActor> CurrentInteractable;

	bool bShouldRotate = true;
private:
	FRotator DesiredRotation;

	// -------------------
	// Not used
	bool bCamSwitch;
	int32 CurrentCamIndex = 0;
	// ------------------
};
