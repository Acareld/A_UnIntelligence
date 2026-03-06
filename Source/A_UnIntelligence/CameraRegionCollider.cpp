// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraRegionCollider.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "CharacterController.h"
#include "CameraRegionListener.h"

// Sets default values
ACameraRegionCollider::ACameraRegionCollider()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	SetRootComponent(Trigger);

	Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Trigger->SetCollisionObjectType(ECC_WorldDynamic);
	Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Trigger->SetGenerateOverlapEvents(true);
}

// Called when the game starts or when spawned
void ACameraRegionCollider::BeginPlay()
{
	Super::BeginPlay();
	
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &ACameraRegionCollider::OnOverlapBegin);
	Trigger->OnComponentEndOverlap.AddDynamic(this, &ACameraRegionCollider::OnOverlapEnd);
}

// Called every frame
void ACameraRegionCollider::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACameraRegionCollider::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACharacterController* Char = Cast<ACharacterController>(OtherActor);
	if (!Char)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cast to CharacterController failed"));
		return;
	}
	if (Char->GetClass()->ImplementsInterface(UCameraRegionListener::StaticClass()))
	{
		ICameraRegionListener::Execute_OnCameraRegionEntered(Char, this);
	}

}

void ACameraRegionCollider::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ACharacterController* Char = Cast<ACharacterController>(OtherActor);
	if (!Char)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cast to CharacterController failed"));
		return;
	}
	if (Char->GetClass()->ImplementsInterface(UCameraRegionListener::StaticClass()))
	{
		ICameraRegionListener::Execute_OnCameraRegionExited(Char, this);
	}
}

