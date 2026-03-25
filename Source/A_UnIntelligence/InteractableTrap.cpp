// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableTrap.h"
#include "CharacterController.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "InspectorGameModeBase.h"
#include "GameplayTagContainer.h"
#include "Components/WidgetComponent.h"
#include "Animation/SkeletalMeshActor.h"
#include "HoverTextWidget.h"
#include "LevelSequencePlayer.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"


#include "Components/CapsuleComponent.h"

// Sets default values
AInteractableTrap::AInteractableTrap()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	SetRootComponent(RootSceneComponent);

	InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
	InteractionVolume->SetupAttachment(RootSceneComponent);

	InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	/*Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootSceneComponent);*/

	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMeshComp->SetupAttachment(RootSceneComponent);

	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMeshComp->SetupAttachment(RootSceneComponent);

	StaticMeshComp->SetVisibility(true);
	SkeletalMeshComp->SetVisibility(true);

	ActiveMeshComp = StaticMeshComp;

	NameWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("NameWidget"));
	NameWidget->SetupAttachment(RootComponent);
	NameWidget->SetWidgetSpace(EWidgetSpace::World);
	NameWidget->SetDrawAtDesiredSize(true);
	NameWidget->SetTwoSided(true);

	NameWidget->SetRelativeLocation(FVector(0.f, -50.f, 80.f));

	NameWidget->SetRelativeScale3D(FVector(40.f));

	LeaderLineDown = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeaderLineDown"));
	LeaderLineDown->SetupAttachment(RootComponent);
	LeaderLineDown->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeaderLineDown->SetGenerateOverlapEvents(false);
	LeaderLineDown->SetCastShadow(false);
	LeaderLineDown->bCastDynamicShadow = false;
	LeaderLineDown->bCastStaticShadow = false;

	LeaderLineDown->SetRelativeLocation(FVector(0.f, 0.f, 40.f));

	LeaderLineSide = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeaderLineSide"));
	LeaderLineSide->SetupAttachment(RootComponent);
	LeaderLineSide->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeaderLineSide->SetGenerateOverlapEvents(false);
	LeaderLineSide->SetCastShadow(false);
	LeaderLineSide->bCastDynamicShadow = false;
	LeaderLineSide->bCastStaticShadow = false;

	LeaderLineSide->SetRelativeLocation(FVector(0.f, 0.f, 40.f));

	InteractionVolume->OnComponentBeginOverlap.AddDynamic(this, &AInteractableTrap::OnOverlapBegin);
	InteractionVolume->OnComponentEndOverlap.AddDynamic(this, &AInteractableTrap::OnOverlapEnd);
}

void AInteractableTrap::BeginPlay()
{
	Super::BeginPlay();
	RefreshActiveMesh();
	CalculateTopWorldCorners();
	CalculateTextAnchorPoints();
	CollectAnimData();

	SetHoverUIVisible(false);

	CurrentAnchorIndex = DefaultAnchorIndex;

	if (UUserWidget* W = NameWidget->GetUserWidgetObject())
	{
		UHoverTextWidget* HoverText = Cast<UHoverTextWidget>(W);
		if (IsValid(TrapDef))
		{
			HoverText->SetTrapName(TrapDef->DisplayName);
		}
	}
}

void AInteractableTrap::RefreshActiveMesh()
{
	ActiveMeshComp = nullptr;

	const bool bHasStaticMesh =
		StaticMeshComp && StaticMeshComp->GetStaticMesh();

	const bool bHasSkeletalMesh =
		SkeletalMeshComp && SkeletalMeshComp->GetSkeletalMeshAsset();

	if (VisualType == ETrapVisualType::SkeletalMesh && bHasSkeletalMesh)
	{
		ActiveMeshComp = SkeletalMeshComp;

		StaticMeshComp->SetVisibility(false);
		StaticMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		SkeletalMeshComp->SetVisibility(true);
		SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	else if (VisualType == ETrapVisualType::StaticMesh && bHasStaticMesh)
	{
		ActiveMeshComp = StaticMeshComp;

		SkeletalMeshComp->SetVisibility(false);
		SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		StaticMeshComp->SetVisibility(true);
		StaticMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	else
	{
		StaticMeshComp->SetVisibility(false);
		StaticMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		SkeletalMeshComp->SetVisibility(false);
		SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AInteractableTrap::Interact_Implementation(APawn* InstigatorPawn)
{
	AnimInstigatorPawn = OverlappingPawn;
	InteractionVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AController* C = InstigatorPawn ? InstigatorPawn->GetController() : nullptr;
	if (!C)
	{
		return;
	}
	ACharacterController* Char = Cast<ACharacterController>(InstigatorPawn);
	if (!Char)
	{
		return;
	}

	if (!AcceptedItems.IsEmpty())
	{

		const FGameplayTag Tag = Char->GetHeldItemTag();
		if (!AcceptedItems.HasTagExact(Tag))
		{
			UE_LOG(LogTemp, Warning, TEXT("Trap needs Tag!"));
			return;
		}

		Char->DeleteHeldItem();
	}
	//Char->GetCharacterMovement()->DisableMovement();

	DisableInput(Cast<APlayerController>(C));

	//bMoveToInteraction = true;

	InstigatorPawn->SetActorLocation(TrapDef->AnimationPosition);
	InstigatorPawn->SetActorRotation(TrapDef->AnimationRotation, ETeleportType::ResetPhysics);

	PlayAnimations(InstigatorPawn);
}

void AInteractableTrap::PlayAnimations(APawn* Pawn)
{

	//Pawn->SetActorLocation(TrapDef->AnimationPosition);
	//Pawn->SetActorRotation(TrapDef->AnimationRotation, ETeleportType::ResetPhysics);

	if (!TrapDef)
	{
		return;
	}
	ACharacter* Char = Cast<ACharacter>(Pawn);

	if (!Char)
	{
		return;
	}

	Char->GetCharacterMovement()->DisableMovement();

	if (TrapDef->PlayOrder == EAnimPlayOrder::PlayerFirst)
	{
		if (PlayerAnim)
		{
			float Length = PlayerAnim->GetPlayLength();

			GetWorld()->GetTimerManager().SetTimer(
				AnimationDelayTimer,
				[this]()
				{
					PlayTrapAnimation(false);
				},
				Length * TrapDef->DelayPercentage,
				false
			);
			if (TrapDef->ReverseAnimAfterDelay)
			{
				GetWorld()->GetTimerManager().SetTimer(
					ReverseAnimTimer,
					[this]()
					{
						PlayTrapAnimation(true);
					},
					Length * TrapDef->ReverseDelayPercentage,
					false
				);
			}

			Char->GetMesh()->PlayAnimation(PlayerAnim, false);
		}
	}
	else if (TrapDef->PlayOrder == EAnimPlayOrder::TrapFirst)
	{

	}
	else if (TrapDef->PlayOrder == EAnimPlayOrder::Simultaneously)
	{
		if (PlayerAnim)
		{
			float Length = PlayerAnim->GetPlayLength();

			Char->GetMesh()->PlayAnimation(PlayerAnim, false);
			PlayTrapAnimation(false);
		}
	}

	
		// Current 2s respawn delay added to the default animation length
		GetWorld()->GetTimerManager().SetTimer(
			DelayedRespawnTimer,
			this,
			&AInteractableTrap::DelayedRespawn,
			MaxAnimLength + 1.f,
			false
		);
	
	
	if (TrapDef->UseVFX)
	{
		GetWorld()->GetTimerManager().SetTimer(
			VFXTimer,
			this,
			&AInteractableTrap::FireVFX,
			MaxAnimLength * TrapDef->EffectDelayPercentage,
			false
		);
	}

}

void AInteractableTrap::PlayTrapAnimation(bool bReverse)
{
	if (TrapSequencePlayer)
	{
		if (bReverse)
		{
			UE_LOG(LogTemp, Warning, TEXT("Reverse triggered"));
			TrapSequencePlayer->PlayReverse();
		}
		else
		{
			TrapSequencePlayer->Play();
		}
	}
	else if (TrapMeshAnim)
	{
		if (USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>(ActiveMeshComp))
		{
			if (TrapDef->AttachToSocket)
			{
				AnimInstigatorPawn->AttachToComponent(SkelComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TrapDef->SocketName);
			}
			SkelComp->PlayAnimation(TrapMeshAnim, false);
			return;
		}

		UE_LOG(LogTemp, Warning, TEXT("TrapMeshAnim is set, but active mesh is not skeletal"));
	}

}


void AInteractableTrap::DelayedRespawn()
{
	

	AController* C = AnimInstigatorPawn.IsValid() ? AnimInstigatorPawn->GetController() : nullptr;
	if (!C)
	{
		return;
	}

	if (!TrapDef->IsTrap)
	{
		if (ACharacter* Char = Cast<ACharacter>(AnimInstigatorPawn.Get()))
		{
			Char->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			Char->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

			if (ACharacterController* CController = Cast<ACharacterController>(Char))
			{
				CController->bShouldRotate = true;
			}

			USkeletalMeshComponent* SkelMesh = Char->GetMesh();
			if (SkelMesh)
			{
				SkelMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
				SkelMesh->SetAnimInstanceClass(Char->GetMesh()->GetAnimClass());
			}
		}

		return;
	}

	if (AInspectorGameModeBase* GM = Cast<AInspectorGameModeBase>(UGameplayStatics::GetGameMode(this)))
	{
		if (ACharacter* SourceChar = Cast<ACharacter>(AnimInstigatorPawn))
		{
			USkeletalMeshComponent* SourceMesh = SourceChar->GetMesh();
			FTransform SpawnTransform = SourceMesh->GetComponentTransform();
			SpawnTransform.SetLocation(FVector(SpawnTransform.GetLocation().X, SpawnTransform.GetLocation().Y, SpawnTransform.GetLocation().Z + TrapDef->FinalPoseZOffset));

			GM->RespawnPlayer(C);

			SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			StaticMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			SpawnFrozenPoseCopy(SourceMesh, SpawnTransform, PlayerAnim);
		}
		

		

		
	}
}

void AInteractableTrap::FireVFX()
{
	if (TrapDef->EffectSystem)
	{
		UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrapDef->EffectSystem,
			GetRootComponent(),
			NAME_None,
			TrapDef->EffectLocation,
			TrapDef->EffectRotation,
			EAttachLocation::Type::KeepRelativeOffset,
			true
		);
		NiagaraComp->SetVariableVec3(FName("SpawnAreaSize"), TrapDef->EffectBoxSize);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Effect to fire"));
	}
}

void AInteractableTrap::SpawnFrozenPoseCopy(USkeletalMeshComponent* SourceMesh, FTransform SpawnTransform, UAnimSequence* PoseAnim)
{
	if (!SourceMesh || !PoseAnim) return;

	UWorld* World = GetWorld();
	if (!World) return;

	
	ASkeletalMeshActor* FrozenActor = World->SpawnActorDeferred<ASkeletalMeshActor>(
		ASkeletalMeshActor::StaticClass(),
		SpawnTransform
	);
	
	if (!FrozenActor) return;

	USkeletalMeshComponent* NewMesh = FrozenActor->GetSkeletalMeshComponent();
	

	if (!NewMesh || !SourceMesh) return;

	NewMesh->SetSkeletalMeshAsset(SourceMesh->GetSkeletalMeshAsset());
	//NewMesh->SetRelativeTransform(SourceMesh->GetRelativeTransform());

	NewMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	NewMesh->SetAnimation(PoseAnim);

	const float EndTime = FMath::Max(0.f, PoseAnim->GetPlayLength() - 0.001f);
	NewMesh->SetPosition(EndTime, false);
	NewMesh->Stop();

	NewMesh->TickAnimation(0.f, false);
	NewMesh->RefreshBoneTransforms();
	NewMesh->UpdateComponentToWorld();
	NewMesh->FinalizeBoneTransform();

	FrozenActor->FinishSpawning(SpawnTransform);

	const FBoxSphereBounds Bounds = NewMesh->CalcBounds(FTransform::Identity);

	UBoxComponent* WalkBox = NewObject<UBoxComponent>(FrozenActor);
	WalkBox->SetupAttachment(NewMesh);
	WalkBox->RegisterComponent();

	FVector Extent = Bounds.BoxExtent;
	Extent.Z = FMath::Min(Extent.Z, 13.f);

	WalkBox->SetRelativeLocation(Bounds.Origin);
	WalkBox->SetRelativeRotation(FRotator::ZeroRotator);
	WalkBox->SetBoxExtent(Extent);

	WalkBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WalkBox->SetCollisionProfileName(TEXT("BlockAll"));
	WalkBox->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_Yes;

	NewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


bool AInteractableTrap::CanPickup(AActor* ByActor)
{
	return false;
}

void AInteractableTrap::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Overlap of trap"));
	ACharacterController* Char = Cast<ACharacterController>(OtherActor);
	if (!Char)

	{
		UE_LOG(LogTemp, Warning, TEXT("Cast to CharacterController failed"));
		return;
	}

	Char->NearbyInteractables.Add(this);

	TextWidgetDefaultPos = NameWidget->GetComponentLocation();
	DesiredAnchor = TextWidgetDefaultPos;
	OverlappingPawn = Cast<APawn>(OtherActor);

	/*if (OverlappingPawn.IsValid())
	{
		GetWorldTimerManager().SetTimer(
			UIUpdateTimer,
			this,
			&AInteractableTrap::UpdateHoverUI,
			0.05f,
			true
		);

		UpdateHoverUI();
	}*/
	bOverlap = true;

}

void AInteractableTrap::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ACharacterController* Char = Cast<ACharacterController>(OtherActor);
	if (!Char) return;

	Char->NearbyInteractables.Remove(this);

	if (OverlappingPawn.Get() == OtherActor)
	{
		OverlappingPawn.Reset();
		GetWorldTimerManager().ClearTimer(UIUpdateTimer);
		NameWidget->SetWorldLocation(TextWidgetDefaultPos);
		DesiredAnchor = TextWidgetDefaultPos;
		LastViableAnchor = DesiredAnchor;
		SetHoverUIVisible(false);
	}

	bOverlap = false;
	CurrentAnchorIndex = DefaultAnchorIndex;
}

EInteractionType AInteractableTrap::GetInteractionType() const
{
	return EInteractionType::Trap;
}

void AInteractableTrap::CalculateTopWorldCorners()
{
	FVector Min, Max;
	if (!GetActiveMeshLocalBounds(Min, Max))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get LocalBounds"));
		return;
	}

	const USceneComponent* BoundsSource = ActiveMeshComp
		? Cast<USceneComponent>(ActiveMeshComp)
		: Cast<USceneComponent>(InteractionVolume);

	if (!BoundsSource)
	{
		return;
	}

	FVector TopLocalCorners[5] =
	{
		FVector(Max.X, Max.Y, Max.Z),
		FVector(Max.X, Min.Y, Max.Z),
		FVector(Min.X, Max.Y, Max.Z),
		FVector(Min.X, Min.Y, Max.Z),
		FVector((Min.X + Max.X) * 0.5f, (Min.Y + Max.Y) * 0.5f, (Min.Z + Max.Z) * 0.5f)
	};

	FTransform T = BoundsSource->GetComponentTransform();

	for (int i = 0; i < 5; i++)
	{
		TopWorldCorners[i] = T.TransformPosition(TopLocalCorners[i]);
	}
	TopWorldCorners[4] = GetActorLocation();
}

bool AInteractableTrap::IsInteractable(AActor* Other)
{
	FVector TargetLoc = Other->GetActorLocation();
	FVector Forward = Other->GetActorForwardVector();
	Forward.Z = 0.f;
	Forward = Forward.GetSafeNormal();

	if (VisualType == ETrapVisualType::None)
	{
		FVector ActorToTrap = GetActorLocation() - TargetLoc;
		ActorToTrap.Z = 0.f;

		float Dot = Forward.Dot(ActorToTrap.GetSafeNormal());
		return Dot > -0.5f;
	}


	float BestDist = FLT_MAX;
	float BestDot = -FLT_MAX;
	FVector ClosestCorner = FVector::ZeroVector;

	for (int i = 0; i < 5; i++)
	{
		FVector ActorToCorner = TopWorldCorners[i] - TargetLoc;
		ActorToCorner.Z = 0.f;
		float Dot = Forward.Dot(ActorToCorner.GetSafeNormal());

		if (Dot > BestDot)
		{
			BestDot = Dot;
			ClosestCorner = TopWorldCorners[i];
		}
	}
	if (BestDot >= 0.71f)
	{
		return true;
	}
	return false;
}


void AInteractableTrap::SetHoverUIVisible(bool bVisible)
{
	if (bUIVisible == bVisible) return;
	bUIVisible = bVisible;

	if (NameWidget)
	{
		NameWidget->SetVisibility(bVisible, true);
	}
	if (LeaderLineDown)
	{
		LeaderLineDown->SetVisibility(bVisible, true);
	}
	if (LeaderLineSide)
	{
		LeaderLineSide->SetVisibility(bVisible, true);
	}
}

void AInteractableTrap::UpdateHoverUI()
{
	APawn* P = OverlappingPawn.Get();
	if (!P)
	{
		UE_LOG(LogTemp, Warning, TEXT("No overlapping actor"));
		SetHoverUIVisible(false);
		return;
	}

	const bool bFacing = IsInteractable(P);
	SetHoverUIVisible(bFacing);

	FVector CamLoc;
	FRotator CamRot;
	if (bFacing && NameWidget)
	{
		APlayerController* PC = Cast<APlayerController>(P->GetController());
		if (PC)
		{
			PC->GetPlayerViewPoint(CamLoc, CamRot);

			NameWidget->SetWorldRotation((CamLoc - NameWidget->GetComponentLocation()).Rotation());
		}
	}
	else
	{
		//bNeedTextSwitch = false;
		//bIsSwitching = false;
		return;
	}
	FVector Start = GetActorLocation();

	if (ActiveMeshComp)
	{
		Start = ActiveMeshComp->Bounds.Origin;
	}
	else if (InteractionVolume)
	{
		Start = InteractionVolume->Bounds.Origin;
	}
	FVector End = NameWidget->GetComponentLocation();

	FHitResult HitResult;
	FVector StartTrace = CamLoc;
	FVector EndTrace = NameWidget->GetComponentLocation();
	FCollisionQueryParams CQP;
	CQP.AddIgnoredActor(this);

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_GameTraceChannel1);
	DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Yellow, false, 0.1f);
	FVector DesiredVector = EndTrace;
	if (GetWorld()->SweepSingleByObjectType(HitResult, StartTrace, EndTrace, FQuat::Identity, ObjectParams, FCollisionShape::MakeSphere(15), CQP))
	{
		//DrawDebugSphere(GetWorld(), StartTrace, 15.f, 16, FColor::Green, false, 0.1f);
		//DrawDebugSphere(GetWorld(), EndTrace, 15.f, 16, FColor::Red, false, 0.1f);
		//DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Yellow, false, 0.1f);
		AActor* Hit = HitResult.GetActor();

		if (Hit && Cast<ACharacterController>(Hit) && !bIsSwitching)
		{
			TargetAnchorIndex = PickViableTextAnchor(CurrentAnchorIndex, CamLoc);

			if (TargetAnchorIndex != CurrentAnchorIndex)
			{
				DesiredAnchor = Anchors[TargetAnchorIndex];
				bNeedTextSwitch = true;
				bIsSwitching = true;
			}
		}
	}

	End.Z -= 5.f;
	End.Y += 5.f;
	Start.X += LeaderLineOffsetX;
	Start.Y += LeaderLineOffsetY;

	FVector DirectionDown = FVector(0, 0, End.Z - Start.Z);
	float Length = DirectionDown.Size();

	FVector MidPoint = End - DirectionDown * 0.5f;
	LeaderLineDown->SetWorldLocation(MidPoint);

	FRotator Rot = FRotationMatrix::MakeFromZ(DirectionDown.GetSafeNormal()).Rotator();
	LeaderLineDown->SetWorldRotation(Rot);

	float MeshHeight = 100.f;
	LeaderLineDown->SetWorldScale3D(FVector(0.02f, 0.02f, Length / MeshHeight));

	FVector DirectionSide = End - Start;
	DirectionSide.Z = 0.f;
	float LengthSide = DirectionSide.Size();

	FVector MidPointSide = Start + DirectionSide * 0.5f;
	LeaderLineSide->SetWorldLocation(MidPointSide);

	FRotator RotSide = FRotationMatrix::MakeFromZ(DirectionSide.GetSafeNormal()).Rotator();
	LeaderLineSide->SetWorldRotation(RotSide);

	LeaderLineSide->SetWorldScale3D(FVector(0.02f, 0.02f, LengthSide / MeshHeight));
}

int32 AInteractableTrap::PickViableTextAnchor(int32 Current, FVector CamLoc)
{
	float Best = FLT_MAX;
	int32 BestIndex = Current;
	for (int32 i = 0; i < Anchors.Num(); ++i)
	{
		if (i == Current)
		{
			continue;
		}

		const float Dist = FVector::DistSquared(Anchors[i], CamLoc);
		if (Dist < Best)
		{
			Best = Dist;
			BestIndex = i;
		}
	}

	return BestIndex;
}

void AInteractableTrap::CalculateTextAnchorPoints()
{
	/*FVector BoundsOrigin;
	FVector BoundsExtent;
	Mesh->GetOwner()->GetActorBounds(false, BoundsOrigin, BoundsExtent);

	float SideOffset = BoundsExtent.Y + 20.f;
	float HeightOffset = BoundsExtent.Z + 35.f;

	const FTransform T = GetActorTransform();*/
	Anchors.Reset();

	Anchors.Add(TopWorldCorners[0]);
	Anchors.Add(TopWorldCorners[1]);
	Anchors.Add(TopWorldCorners[2]);
}

void AInteractableTrap::CollectAnimData()
{
	if (!TrapDef)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Trap Definition found!"));
		return;
	}

	float Length = FLT_MIN;
	float TrapAnimLength = FLT_MIN;

	if (TrapDef->PlayerAnim)
	{
		PlayerAnim = Cast<UAnimSequence>(TrapDef->PlayerAnim);
		Length = PlayerAnim->GetPlayLength();
	}
	if (TrapDef->TrapMeshAnim)
	{
		TrapMeshAnim = Cast<UAnimSequence>(TrapDef->TrapMeshAnim);
		TrapAnimLength = TrapMeshAnim->GetPlayLength();

	}
	else if (TrapDef->TrapSequence)
	{
		ALevelSequenceActor* SequenceActor;
		FMovieSceneSequencePlaybackSettings PlayerSettings = FMovieSceneSequencePlaybackSettings();
		PlayerSettings.FinishCompletionStateOverride = EMovieSceneCompletionModeOverride::ForceKeepState;

		TrapSequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
			GetWorld(),
			TrapDef->TrapSequence,
			PlayerSettings,
			SequenceActor);

		UMovieScene* MovieScene = TrapDef->TrapSequence->GetMovieScene();
		if (!MovieScene)
		{
			UE_LOG(LogTemp, Warning, TEXT("Sequence contains no MovieScene"));
			return;
		}

		const FFrameRate FrameRate = MovieScene->GetTickResolution();
		const TRange<FFrameNumber> Range = MovieScene->GetPlaybackRange();

		const int32 StartFrame = Range.GetLowerBoundValue().Value;
		const int32 EndFrame = Range.GetUpperBoundValue().Value;

		const int32 FrameCount = EndFrame - StartFrame;

		TrapAnimLength = FrameCount / FrameRate.AsDecimal();
		if (TrapDef->ReverseAnimAfterDelay)
		{
			TrapAnimLength *= 2;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Animation or Sequence for the trap found!"));
		MaxAnimLength = Length;
		return;
	}


	if (TrapDef->PlayOrder == EAnimPlayOrder::Simultaneously)
	{
		if (Length < TrapAnimLength)
		{
			Length = TrapAnimLength;
		}
	}
	else if (TrapDef->PlayOrder == EAnimPlayOrder::PlayerFirst)
	{
		float TempLength = Length;
		if (TrapDef->ReverseAnimAfterDelay)
		{
			TempLength += TrapAnimLength - (Length * (1 - TrapDef->ReverseDelayPercentage));
		}
		else
		{
			TempLength += TrapAnimLength - (Length * (1 - TrapDef->DelayPercentage));
		}
		if (Length < TempLength)
		{
			Length = TempLength;
		}
	}

	MaxAnimLength = Length;
}

void AInteractableTrap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bMoveToInteraction)
	{
		FVector ToTarget = TrapDef->StartAnimationPosition - AnimInstigatorPawn->GetActorLocation();
		ToTarget.Z = 0.f;

		float Distance = ToTarget.Size();

		ACharacter* C = Cast<ACharacter>(AnimInstigatorPawn);

		if (Distance > 10.f)
		{
			ToTarget.Normalize();
			C->AddMovementInput(ToTarget, 1.0f);
			C->SetActorRotation(FMath::RInterpTo(C->GetActorRotation(), ToTarget.Rotation(), DeltaTime, 10.f));
		}
		else
		{
			bMoveToInteraction = false;
			//C->GetCharacterMovement()->StopMovementImmediately();



			PlayAnimations(AnimInstigatorPawn.Get());
		};
	}



	if (bOverlap)
	{
		UpdateHoverUI();

		if (bNeedTextSwitch)
		{
			FVector CurrentTextPos = NameWidget->GetComponentLocation();

			FVector NewLocation = FMath::VInterpTo(
				CurrentTextPos,
				DesiredAnchor,
				DeltaTime,
				6.f
			);

			NameWidget->SetWorldLocation(NewLocation);

			if (NewLocation.Equals(DesiredAnchor, 1.0f))
			{
				CurrentAnchorIndex = TargetAnchorIndex;
				bNeedTextSwitch = false;
				bIsSwitching = false;
			}
		}
	}
}

