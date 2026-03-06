// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CameraRegionListener.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCameraRegionListener : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class A_UNINTELLIGENCE_API ICameraRegionListener
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Camera Region")
	void OnCameraRegionEntered(ACameraRegionCollider* Region);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Camera Region")
	void OnCameraRegionExited(ACameraRegionCollider* Region);
};
