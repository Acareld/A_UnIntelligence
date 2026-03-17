// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "CameraRegionListener.h"
#include "CharacterController.generated.h"

class ACameraRegionCollider;

USTRUCT()
struct FActiveRegion
{
	GENERATED_BODY()

	UPROPERTY() TObjectPtr<ACameraRegionCollider> Region = nullptr;
	UPROPERTY() double EnterTime = 0.0;
};

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
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	//void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	void Move(const FInputActionValue& Value);
	virtual void PossessedBy(AController* NewController) override;
	virtual void PawnClientRestart() override;
	void ApplyInputMapping();
	void CamSwitch();
	void Interact();
	AActor* GetBestInteractable() const;
	ACameraRegionCollider* ChooseBestRegion() const;
	void RecomputeAndApplyCamera();
	void Pickup(AActor* Item);
	void Drop();
	void DropPressed();
	void Countdown();

	UPROPERTY() 
	TArray<FActiveRegion> ActiveRegions;

	UPROPERTY(EditAnywhere, Category = "Input")
	TSoftObjectPtr<UInputMappingContext> InputMapping;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* CamAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* InteractAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* DropAction;

	UPROPERTY(EditAnywhere)
	UAnimationAsset* RespawnAnim;

	UPROPERTY(EditAnywhere)
	FTransform RespawnTransform;


	UPROPERTY(EditAnywhere, Category = "Region")
	AActor* DefaultViewTarget;

	UPROPERTY()
	TArray<TObjectPtr<class ACameraActor>> LevelCameras;

	FVector Forward = FVector(1.f, 0.f, 0.f);
	FVector Right = FVector(0.f, 1.f, 0.f);

	UPROPERTY()
	AActor* HeldItem = nullptr;

	UPROPERTY(BlueprintReadOnly)
	int Seconds = 15;
	UPROPERTY(BlueprintReadOnly)
	int Minutes = 5;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	int32 GetCurrentCamIndex() const { return CurrentCamIndex; };
	void SetCurrentCamIndex(int32 NewCamIndex) { CurrentCamIndex = NewCamIndex; };
	void SetCorrectViewTarget();
	void AutoCamSwitch();

	void SetRespawnTransform(const FTransform& NewTransform);
	FTransform GetRespawnTransform() const { return RespawnTransform; }
	FGameplayTag GetHeldItemTag() const;
	void DeleteHeldItem();

	void PlayAnimation(UAnimationAsset* Anim);
	void HandleRespawn();

	UPROPERTY()
	TSet<TObjectPtr<AActor>> NearbyInteractables;

	UPROPERTY()
	TObjectPtr<AActor> CurrentInteractable;
private:
	bool bCamSwitch;

	int32 CurrentCamIndex = 0;

	

	




};
