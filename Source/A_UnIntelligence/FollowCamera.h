// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "FollowCamera.generated.h"

/**
 * 
 */
UCLASS()
class A_UNINTELLIGENCE_API AFollowCamera : public ACameraActor
{
	GENERATED_BODY()
public:
	AFollowCamera();
	virtual void Tick(float DeltaTime) override;
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow")
	TObjectPtr<AActor> TargetActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow")
	float InterpSpeed = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow")
	float ClampRight = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow")
	float ClampLeft = -60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow")
	float ClampUp = -20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow")
	float ZoomInterpSpeed = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow")
	float ZoomDistance = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Follow")
	float DesiredFOV = 60.f;

	UCameraComponent* CameraComponent;

	float DefaultFOV = 90.f;

	bool bHasZoomed = false;
};
