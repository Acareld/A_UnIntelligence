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
#include "HoverTextWidget.h"
#include "LevelSequencePlayer.h"

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

	StaticMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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
	Char->GetCharacterMovement()->DisableMovement();
	InstigatorPawn->SetActorLocation(TrapDef->AnimationPosition);
	InstigatorPawn->SetActorRotation(TrapDef->AnimationRotation, ETeleportType::ResetPhysics);
	PlayTrapSequence(InstigatorPawn);
}

void AInteractableTrap::PlayTrapSequence(APawn* Pawn)
{

	if (!TrapDef)
	{
		return;
	}
	ACharacter* Char = Cast<ACharacter>(Pawn);
	if (!Char)
	{
		return;
	}
	if (TrapDef->PlayerAnim)
	{
		UAnimSequence* AnimSequence = Cast<UAnimSequence>(TrapDef->PlayerAnim);
		float Length = AnimSequence->GetPlayLength();

		GetWorld()->GetTimerManager().SetTimer(
			AnimationDelayTimer,
			this,
			&AInteractableTrap::PlayTrapAnimationDelayed,
			Length + 0.1f,
			false
		);
		Char->GetMesh()->PlayAnimation(TrapDef->PlayerAnim, false);

	}

	
}

void AInteractableTrap::PlayTrapAnimationDelayed()
{
	if (TrapDef->TrapSequence)
	{
		ALevelSequenceActor* SequenceActor;
		FMovieSceneSequencePlaybackSettings PlayerSettings = FMovieSceneSequencePlaybackSettings();
		PlayerSettings.FinishCompletionStateOverride = EMovieSceneCompletionModeOverride::ForceKeepState;
		ULevelSequencePlayer* LevelSequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
			GetWorld(),
			TrapDef->TrapSequence,
			PlayerSettings,
			SequenceActor);

		LevelSequencePlayer->OnFinished.AddDynamic(this, &AInteractableTrap::DelayedRespawn);
		LevelSequencePlayer->Play();
	}
	else if (TrapDef->TrapMeshAnim)
	{
		if (USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>(ActiveMeshComp))
		{

			UAnimSequence* AnimSequence = Cast<UAnimSequence>(TrapDef->TrapMeshAnim);
			float Length = AnimSequence->GetPlayLength();

			GetWorld()->GetTimerManager().SetTimer(
				AnimationDelayTimer,
				this,
				&AInteractableTrap::DelayedRespawn,
				Length,
				false
			);
			SkelComp->PlayAnimation(TrapDef->TrapMeshAnim, false);

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
	if (AInspectorGameModeBase* GM = Cast<AInspectorGameModeBase>(UGameplayStatics::GetGameMode(this)))
	{
		
		GM->RespawnPlayer(C);
		
		SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		StaticMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
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
	//bNeedTextSwitch = false;
	//bIsSwitching = false;
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
		DrawDebugSphere(GetWorld(), StartTrace, 15.f, 16, FColor::Green, false, 0.1f);
		DrawDebugSphere(GetWorld(), EndTrace, 15.f, 16, FColor::Red, false, 0.1f);
		DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Yellow, false, 0.1f);
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


void AInteractableTrap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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




void AInteractableTrap::BeginFallOver(AController* ControllerToRespawn)
{
	// Prevent spamming
	if (bIsFalling)
	{
		return;
	}

	PendingController = ControllerToRespawn;

	bIsFalling = true;
	FallElapsed = 0.f;
	FallStartRot = GetActorRotation();
	FallTargetRot = FallStartRot + FallDelta;

	if (ActiveMeshComp)
	{
		ActiveMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	InteractionVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true);
	}


	GetWorldTimerManager().SetTimer(
		FallTickTimer,
		this,
		&AInteractableTrap::UpdateFall,
		1.0f / 60.0f,
		true
	);


	GetWorldTimerManager().SetTimer(
		FallFinishedTimer,
		this,
		&AInteractableTrap::FinishFall,
		FallDuration,
		false
	);
}

void AInteractableTrap::UpdateFall()
{
	if (!bIsFalling)
		return;

	const float Step = GetWorldTimerManager().GetTimerRate(FallTickTimer);
	FallElapsed += Step;

	const float Alpha = FMath::Clamp(FallElapsed / FallDuration, 0.f, 1.f);
	const float SmoothAlpha = FMath::InterpEaseIn(0.f, 1.f, Alpha, 3.0f);

	const FRotator NewRot = FMath::Lerp(FallStartRot, FallTargetRot, SmoothAlpha);
	SetActorRotation(NewRot);

	if (Alpha >= 0.8 && !bDidSnap)
	{
		bDidSnap = true;
		ACharacter* Char = UGameplayStatics::GetPlayerCharacter(this, 0);
		FVector CurPos = Char->GetActorLocation();
		const float HalfHeight = Char->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		const float GroundZ = CurPos.Z - HalfHeight;
		const float YHalfHeight = Char->GetMesh()->Bounds.BoxExtent.Y;
		FVector Pos = FVector(CurPos.X, CurPos.Y, 10);
		FRotator CurRot = Char->GetActorRotation();
		CurRot.Pitch += 90.f;

		Char->SetActorRotation(CurRot);
		Char->SetActorLocation(Pos);
	}

	if (Alpha >= 1.f)
	{
		GetWorldTimerManager().ClearTimer(FallTickTimer);
	}
}

void AInteractableTrap::FinishFall()
{

	GetWorldTimerManager().ClearTimer(FallTickTimer);
	bIsFalling = false;
	SetActorRotation(FallTargetRot);



	GetWorldTimerManager().SetTimer(
		RespawnDelayTimer,
		this,
		&AInteractableTrap::DoDelayedRespawn,
		1.5f,
		false
	);

}

void AInteractableTrap::DoDelayedRespawn()
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		PC->SetIgnoreMoveInput(false);
		PC->SetIgnoreLookInput(false);
	}

	if (PendingController.IsValid())
	{
		if (AInspectorGameModeBase* GM = Cast<AInspectorGameModeBase>(UGameplayStatics::GetGameMode(this)))
		{
			GM->RespawnPlayer(PendingController.Get());
		}
	}
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PendingController.Reset();
}

