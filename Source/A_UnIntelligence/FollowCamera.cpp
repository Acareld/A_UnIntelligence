// Fill out your copyright notice in the Description page of Project Settings.


#include "FollowCamera.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Camera/CameraComponent.h"

AFollowCamera::AFollowCamera()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AFollowCamera::BeginPlay()
{
	Super::BeginPlay();

	if (!TargetActor)
	{
		TargetActor = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	}

	CameraComponent = GetCameraComponent();
}

void AFollowCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn) return;

	TargetActor = PlayerPawn;

	FVector CamLoc = GetActorLocation();
	FVector ActorLoc = TargetActor->GetActorLocation();

	// Calculate Desired Camera rotation
	FRotator DesiredRot = UKismetMathLibrary::FindLookAtRotation(CamLoc, ActorLoc);

	// Flatten cam and actor locations for homogenous distance
	CamLoc.Z = 0.f;
	ActorLoc.Z = 0.f;

	const FVector DistanceVec = ActorLoc - CamLoc;
	const float Distance = DistanceVec.SquaredLength();

	// disable roll
	DesiredRot.Roll = 0.0f;


	if (InterpSpeed <= 0.f)
	{
		SetActorRotation(DesiredRot);
	}
	else
	{
		const float Alpha = FMath::GetMappedRangeValueClamped(
			FVector2D(FMath::Square(ZoomDistance), FMath::Square(750.f)),
			FVector2D(0.f, 1.f),
			Distance
		);

		const float TargetFOV = FMath::Lerp(90.f, 40.f, Alpha);

		const float CurrentFOV = CameraComponent->FieldOfView;
		const float NewFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, ZoomInterpSpeed);

		CameraComponent->SetFieldOfView(NewFOV);
		
		float CenterYaw = -90.f; 

		float DeltaYaw = FMath::FindDeltaAngleDegrees(CenterYaw, DesiredRot.Yaw);

		
		DeltaYaw = FMath::Clamp(DeltaYaw, ClampLeft, ClampRight);
		DesiredRot.Yaw = FRotator::NormalizeAxis(CenterYaw + DeltaYaw);
		
		const FRotator Current = GetActorRotation();

		const FRotator SmoothedRot = FMath::RInterpTo(Current, DesiredRot, DeltaTime, InterpSpeed);
		SetActorRotation(SmoothedRot);
	}

	


}