// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "TaggedCameraActor.generated.h"

/**
 * 
 */
UCLASS()
class A_UNINTELLIGENCE_API ATaggedCameraActor : public ACameraActor
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera")
	int32 Id;
};
