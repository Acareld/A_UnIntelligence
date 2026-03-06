// Fill out your copyright notice in the Description page of Project Settings.


#include "CamSwitchCollider.h"
#include "CharacterController.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ACamSwitchCollider::ACamSwitchCollider()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
	InteractionVolume->SetupAttachment(RootComponent);

	InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	InteractionVolume->OnComponentBeginOverlap.AddDynamic(this, &ACamSwitchCollider::OnOverlapBegin);
	//InteractionVolume->OnComponentEndOverlap.AddDynamic(this, &AInteractableTrap::OnOverlapEnd);
}


void ACamSwitchCollider::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Overlap of AutoCamSwitch"));
	ACharacterController* Char = Cast<ACharacterController>(OtherActor);
	if (!Char)

	{
		UE_LOG(LogTemp, Warning, TEXT("Cast to CharacterController failed"));
		return;
	}

	Char->AutoCamSwitch();
}


// Called when the game starts or when spawned
void ACamSwitchCollider::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACamSwitchCollider::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

