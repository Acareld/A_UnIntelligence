// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupItem.h"
#include "CharacterController.h"
#include "Components/BoxComponent.h"

// Sets default values
APickupItem::APickupItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
	RootComponent = InteractionVolume;

	InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetSimulatePhysics(false);

	InteractionVolume->OnComponentBeginOverlap.AddDynamic(this, &APickupItem::OnOverlapBegin);
	InteractionVolume->OnComponentEndOverlap.AddDynamic(this, &APickupItem::OnOverlapEnd);
}

// Called when the game starts or when spawned
void APickupItem::BeginPlay()
{
	Super::BeginPlay();

	InitialRotation = GetActorRotation();
	
}

// Called every frame
void APickupItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool APickupItem::CanPickup(AActor* ByActor)
{
	return !bIsHeld && IsValid(ByActor);
}

void APickupItem::OnPickup(AActor* ByActor)
{
	bIsHeld = true;

	
}

void APickupItem::OnDrop(AActor* ByActor)
{
	bIsHeld = false;
	SetActorRotation(FRotator::ZeroRotator);
}

void APickupItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Overlap of PickupItem"));
	ACharacterController* Char = Cast<ACharacterController>(OtherActor);
	if (!Char)

	{
		UE_LOG(LogTemp, Warning, TEXT("Cast to CharacterController failed"));
		return;
	}

	Char->NearbyInteractables.Add(this);
}

void APickupItem::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ACharacterController* Char = Cast<ACharacterController>(OtherActor);
	if (!Char) return;

	Char->NearbyInteractables.Remove(this);
}

EInteractionType APickupItem::GetInteractionType() const
{
	return EInteractionType::Pickup;
}

