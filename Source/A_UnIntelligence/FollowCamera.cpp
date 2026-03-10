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

	const FVector CamLoc = GetActorLocation();
	const FVector ActorLoc = TargetActor->GetActorLocation();

	const FVector DistanceVec = ActorLoc - CamLoc;
	const float Distance = DistanceVec.Length();

	FRotator DesiredRot = UKismetMathLibrary::FindLookAtRotation(CamLoc, ActorLoc);

	DesiredRot.Roll = 0.0f;

	

	

	if (InterpSpeed <= 0.f)
	{
		SetActorRotation(DesiredRot);
	}
	else
	{
		bool bShouldZoom = Distance > ZoomDistance;

		float TargetFOV = bShouldZoom ? DesiredFOV : DefaultFOV;

		const float CurrentFOV = CameraComponent->FieldOfView;
		const float NewFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, ZoomInterpSpeed);

		CameraComponent->SetFieldOfView(NewFOV);

		DesiredRot.Pitch = bShouldZoom ? DesiredRot.Pitch : FMath::Clamp(DesiredRot.Pitch, -90.f, ClampUp);

		DesiredRot.Yaw = FMath::Clamp(DesiredRot.Yaw, ClampLeft, ClampRight);

		const FRotator Current = GetActorRotation();
		const FRotator SmoothedRot = FMath::RInterpTo(Current, DesiredRot, DeltaTime, InterpSpeed);
		SetActorRotation(SmoothedRot);
	}

	


}