// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterController.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraActor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Interactable.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "CameraRegionCollider.h"
#include "PickupItem.h"
#include "InteractableTrap.h"
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"



// Sets default values
ACharacterController::ACharacterController()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bIgnoreBaseRotation = true;

	GetCharacterMovement()->MaxWalkSpeed = 150.f;
	GetCharacterMovement()->MaxAcceleration = 300.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 75.f;
	GetCharacterMovement()->bUseSeparateBrakingFriction = true;
	GetCharacterMovement()->GroundFriction = 4.f;
	GetCharacterMovement()->BrakingFriction = 0.01f;
	GetCharacterMovement()->BrakingFrictionFactor = 1.0f;
	bCamSwitch = false;

	DesiredRotation = GetActorRotation();
}

void ACharacterController::PawnClientRestart()
{
	Super::PawnClientRestart();
	ApplyInputMapping();
}

// Called every frame
void ACharacterController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bShouldRotate)
	{
		FRotator Rotation = FMath::RInterpTo(GetActorRotation(), DesiredRotation, DeltaTime, 10.f);
		SetActorRotation(Rotation);
	}
	
}
void ACharacterController::SetRotation(FRotator Rotation)
{
	DesiredRotation = Rotation;
}

void ACharacterController::ApplyInputMapping()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) { UE_LOG(LogTemp, Warning, TEXT("No PlayerController")); return; }

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP) { UE_LOG(LogTemp, Warning, TEXT("No LocalPlayer")); return; }

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP);
	if (!Subsystem) { UE_LOG(LogTemp, Warning, TEXT("No EnhancedInput subsystem")); return; }

	if (InputMapping.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("InputMapping is NULL (not assigned)"));
		return;
	}

	UInputMappingContext* Ctx = InputMapping.LoadSynchronous();
	if (!Ctx)
	{
		UE_LOG(LogTemp, Warning, TEXT("InputMapping assigned but failed to load"));
		return;
	}

	Subsystem->AddMappingContext(Ctx, 0);
}


void ACharacterController::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACharacterController::Move);
		//EIC->BindAction(CamAction, ETriggerEvent::Triggered, this, &ACharacterController::CamSwitch);
		EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &ACharacterController::Interact);
		//EIC->BindAction(DropAction, ETriggerEvent::Started, this, &ACharacterController::DropPressed);
	}
}

void ACharacterController::Move(const FInputActionValue& Value)
{

	FVector2D Input = Value.Get<FVector2D>();

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;
	AActor* CurrentViewTarget = PC->GetViewTarget();

	Forward = CurrentViewTarget->GetActorForwardVector();
	Right = CurrentViewTarget->GetActorRightVector();

	Forward.Z = 0.f;
	Right.Z = 0.f;

	Forward.Normalize();
	Right.Normalize();

	FVector MoveDir = (Forward * Input.Y) + (Right * Input.X);
	MoveDir.Z = 0.f;

	if (!MoveDir.IsNearlyZero())
	{
		MoveDir.Normalize();
		FRotator rot = MoveDir.Rotation();

		if (!GetMesh()->GetAnimInstance()->IsAnyMontagePlaying())
		{
			if (GetCharacterMovement()->MovementMode != MOVE_None)
			{
				DesiredRotation = MoveDir.Rotation();
			}			
			AddMovementInput(MoveDir, 1.f);
		}	
	}
}


void ACharacterController::Interact()
{
	AActor* Target = GetBestInteractable();
	
	if (!Target)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Target to interact"));
		return;
	}
	if (IInteractable* Interactable = Cast<IInteractable>(Target))
	{
		// Check for PickupItem or InteractableTrap
		// pickup was removed from game design at this stage, only interactables anymore
		if (Interactable->GetInteractionType() == EInteractionType::Pickup)
		{
			Pickup(Target);
			UE_LOG(LogTemp, Warning, TEXT("Pickup Item"));
		} 
		else 
		{
			AInteractableTrap* Trap = Cast<AInteractableTrap>(Target);

			if (Trap->IsInteractable(this)) 
			{ 
				bShouldRotate = false;
				IInteractable::Execute_Interact(Target, this);
			} 
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Player in Collider but not facing it enough"));
			}
		}
	}
}

AActor* ACharacterController::GetBestInteractable() const
{
	AActor* Best = nullptr;
	float BestDist = FLT_MAX;

	const FVector Loc = GetActorLocation();

	for (AActor* A : NearbyInteractables)
	{
		if (!IsValid(A)) continue;
		if (!A->GetClass()->ImplementsInterface(UInteractable::StaticClass())) continue;
		AInteractableTrap* Trap = Cast<AInteractableTrap>(A);
		
		if (!Trap) continue;
		// early return with first interactable that is in range and character is facing it
		if (Trap->IsInteractable(A))
		{
			return A;
		}

		const float DistSq = FVector::DistSquared(Loc, A->GetActorLocation());
		if (DistSq < BestDist)
		{
			BestDist = DistSq;
			Best = A;
		}
	}
	return Best;
}


// --------------------------------------------------------------------
// Everything below this line was considered in the design process but did
// not make it into the final game

void ACharacterController::SetRespawnTransform(const FTransform& NewTransform)
{
	RespawnTransform = NewTransform;
}

void ACharacterController::CamSwitch()
{
	UE_LOG(LogTemp, Warning, TEXT("CamSwitch CALLED"));

	if (LevelCameras.Num() < 2) return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	CurrentCamIndex = (CurrentCamIndex + 1) % LevelCameras.Num();
	PC->SetViewTargetWithBlend(LevelCameras[CurrentCamIndex], 0.25f);
}

FGameplayTag ACharacterController::GetHeldItemTag() const
{
	APickupItem* Item = Cast<APickupItem>(HeldItem);
	if (!Item) return FGameplayTag();
	return Item->ItemTag;
}

void ACharacterController::DeleteHeldItem()
{
	if (!HeldItem) return;
	HeldItem->Destroy();
}

void ACharacterController::SetCorrectViewTarget()
{
	if (LevelCameras.Num() < 2) return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;
	
	//PC->SetViewTarget(LevelCameras[CurrentCamIndex]);
	PC->SetViewTarget(DefaultViewTarget);
}



void ACharacterController::PlayAnimation(UAnimationAsset* Anim)
{

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

}

void ACharacterController::HandleRespawn()
{
	SetActorTransform(RespawnTransform);

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		//MoveComp->StopMovementImmediately();
		//MoveComp->DisableMovement();
		MoveComp->SetMovementMode(MOVE_Walking);
	}
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		//Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (RespawnAnim)
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			UAnimSequenceBase* AnimSequence = Cast<UAnimSequenceBase>(RespawnAnim);

			float Length = AnimSequence->GetPlayLength();
			AnimInstance->PlaySlotAnimationAsDynamicMontage(
				AnimSequence,
				FName("DefaultSlot"),
				0.1f,
				0.1f,
				1.0f,
				1
			);
		
		}
	}

	if (RespawnAnim)
	{
		const float Duration = RespawnAnim->GetPlayLength();

		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(
			TimerHandle,
			[this]()
			{
				if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
				{
					MoveComp->SetMovementMode(MOVE_Walking);
					if (UCapsuleComponent* Capsule = GetCapsuleComponent())
					{
						Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					}
					
					//SetActorLocation(FVector(-52.76, -296.15, 92), false, nullptr, ETeleportType::ResetPhysics);
				}
			},
			Duration,
			false
		);
	}
	else
	{
		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->SetMovementMode(MOVE_Walking);
		}
	}
}

//Deprecated
void ACharacterController::AutoCamSwitch()
{
	CamSwitch();
}

// Not used
void ACharacterController::OnCameraRegionEntered_Implementation(ACameraRegionCollider* Region)
{
	if (!Region) return;

	// Avoid duplicates
	for (const FActiveRegion& AR : ActiveRegions)
	{
		if (AR.Region == Region) return;
	}

	FActiveRegion NewEntry;
	NewEntry.Region = Region;
	NewEntry.EnterTime = FPlatformTime::Seconds();
	ActiveRegions.Add(NewEntry);

	RecomputeAndApplyCamera();
}

// Not used
void ACharacterController::OnCameraRegionExited_Implementation(ACameraRegionCollider* Region)
{
	if (!Region) return;

	ActiveRegions.RemoveAll([&](const FActiveRegion& AR)
		{
			return AR.Region == Region;
		});

	RecomputeAndApplyCamera();
}

ACameraRegionCollider* ACharacterController::ChooseBestRegion() const
{
	ACameraRegionCollider* Best = nullptr;
	int32 BestPriority = TNumericLimits<int32>::Lowest();


	for (const FActiveRegion& AR : ActiveRegions)
	{
		if (!AR.Region) continue;

		const int32 P = AR.Region->Priority;
		if (!Best || P > BestPriority)
		{
			Best = AR.Region;
			BestPriority = P;
		}
	}
	return Best;
}

// Not used
void ACharacterController::RecomputeAndApplyCamera()
{
	ACameraRegionCollider* BestRegion = ChooseBestRegion();

	AActor* Target = LevelCameras[0];
	float Blend = 0.0f;

	if (BestRegion)
	{
		if (BestRegion->ViewTarget)
		{
			Target = BestRegion->ViewTarget;
			Blend = BestRegion->Blend;
		}
	}

	if (!Target) return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;
	PC->SetViewTargetWithBlend(Target, Blend);
}

void ACharacterController::Pickup(AActor* Item)
{
	if (!Item) return;

	if (IInteractable* Interactable = Cast<IInteractable>(Item))
	{
		Interactable->OnPickup(this);

		// attach item to socket WIP
		const FName Socket = TEXT("hand_r");;//Interactable->GetAttachSocketName();

		Item->AttachToComponent(
			GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			Socket
		);

		UE_LOG(LogTemp, Warning, TEXT("Attached PickupItem"));
	}

	HeldItem = Item;
}

void ACharacterController::DropPressed()
{
	Drop();
}

void ACharacterController::Drop()
{
	if (!HeldItem) return;

	AActor* Item = HeldItem;
	HeldItem = nullptr;

	// Detach
	Item->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	UE_LOG(LogTemp, Warning, TEXT("Drop Item"));
	FVector ActorPos = GetActorLocation();

	const float HalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float GroundZ = GetActorLocation().Z - HalfHeight;
	const float ItemHalfHeight = Cast<APickupItem>(Item)->Mesh->Bounds.BoxExtent.Z;

	Item->SetActorLocation(FVector(ActorPos.X, ActorPos.Y, GroundZ + ItemHalfHeight));

	if (IInteractable* Interactable = Cast<IInteractable>(Item))
	{
		Interactable->OnDrop(this);
	}
}







