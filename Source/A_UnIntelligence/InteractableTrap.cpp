// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableTrap.h"
#include "CharacterController.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "InspectorGameModeBase.h"
#include "GameplayTagContainer.h"
#include "Components/WidgetComponent.h"
#include "HoverTextWidget.h"

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

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(RootSceneComponent);

    NameWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("NameWidget"));
    NameWidget->SetupAttachment(RootComponent);
    NameWidget->SetWidgetSpace(EWidgetSpace::World);
    NameWidget->SetDrawAtDesiredSize(true);
    NameWidget->SetTwoSided(true);

    NameWidget->SetRelativeLocation(FVector(0.f, -50.f, 80.f));
    NameWidget->SetRelativeScale3D(FVector(40.f));

    LeaderLine = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeaderLine"));
    LeaderLine->SetupAttachment(RootComponent);
    LeaderLine->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    LeaderLine->SetGenerateOverlapEvents(false);
    LeaderLine->SetCastShadow(false);
    LeaderLine->bCastDynamicShadow = false;
    LeaderLine->bCastStaticShadow = false;

    LeaderLine->SetRelativeLocation(FVector(0.f, 0.f, 40.f));

    InteractionVolume->OnComponentBeginOverlap.AddDynamic(this, &AInteractableTrap::OnOverlapBegin);
    InteractionVolume->OnComponentEndOverlap.AddDynamic(this, &AInteractableTrap::OnOverlapEnd);
}

void AInteractableTrap::BeginPlay()
{
    Super::BeginPlay();
    CalculateTopWorldCorners();

    SetHoverUIVisible(false);

    if (UUserWidget* W = NameWidget->GetUserWidgetObject())
    {
        UHoverTextWidget* HoverText = Cast<UHoverTextWidget>(W);
        if (IsValid(TrapDef))
        {
            HoverText->SetTrapName(TrapDef->DisplayName);
        }
    }
}

void AInteractableTrap::Interact_Implementation(APawn* InstigatorPawn)
{
    AController* C = InstigatorPawn->GetController();
    if (!C) return;
   

    if (!AcceptedItems.IsEmpty())
    {
        ACharacterController* Char = Cast<ACharacterController>(InstigatorPawn);
        FGameplayTag Tag = Char->GetHeldItemTag();
        if (!AcceptedItems.HasTagExact(Tag))
        {
            UE_LOG(LogTemp, Warning, TEXT("Trap needs Tag!"));
            return;
        } 
        else
        {
            Char->DeleteHeldItem();
        }
    }

    AInspectorGameModeBase* GM = Cast<AInspectorGameModeBase>(UGameplayStatics::GetGameMode(this));
    if (GM)
    {
        GM->RespawnPlayer(C);
    }
    
    //BeginFallOver(C);
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

    OverlappingPawn = Cast<APawn>(OtherActor);
    if (OverlappingPawn.IsValid())
    {
        GetWorldTimerManager().SetTimer(
            UIUpdateTimer,
            this,
            &AInteractableTrap::UpdateHoverUI,
            0.05f,
            true
        );

        UpdateHoverUI(); 
    }
   
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
        SetHoverUIVisible(false);
    }
}

EInteractionType AInteractableTrap::GetInteractionType() const
{
    return EInteractionType::Trap;
}

void AInteractableTrap::CalculateTopWorldCorners()
{
    FVector Min, Max;
    Mesh->GetLocalBounds(Min,Max);

    FVector TopLocalCorners[5] =
    {
        FVector(Max.X, Max.Y, Max.Z),
        FVector(Max.X, Min.Y, Max.Z),
        FVector(Min.X, Max.Y, Max.Z),
        FVector(Min.X, Min.Y, Max.Z),
        FVector(Max.X / 2, Max.Y / 2, Max.Z / 2)
    };

    FTransform T = Mesh->GetComponentTransform();

    for (int i = 0; i < 5; i++)
    {
        TopWorldCorners[i] = T.TransformPosition(TopLocalCorners[i]);
    }
}

bool AInteractableTrap::IsInteractable(AActor* Other)
{
    FVector TargetLoc = Other->GetActorLocation();
    FVector Forward = Other->GetActorForwardVector();
    Forward.Z = 0.f;

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
    if (LeaderLine)
    {
        LeaderLine->SetVisibility(bVisible, true);
    }
}

void AInteractableTrap::UpdateHoverUI()
{
    APawn* P = OverlappingPawn.Get();
    if (!P)
    {
        SetHoverUIVisible(false);
        return;
    }

    const bool bFacing = IsInteractable(P);
    SetHoverUIVisible(bFacing);
    
    if (bFacing && NameWidget)
    {
        APlayerController* PC = Cast<APlayerController>(P->GetController());
        if (PC)
        {
            FVector CamLoc;
            FRotator CamRot;
            PC->GetPlayerViewPoint(CamLoc, CamRot);

            NameWidget->SetWorldRotation((CamLoc - NameWidget->GetComponentLocation()).Rotation());
        }
    }
    FVector Start = GetActorLocation(); 
    FVector End = NameWidget->GetComponentLocation();
    End.Z -= 5.f;
    FVector Direction = End - Start;
    float Length = Direction.Size();
    FVector MidPoint = Start + Direction * 0.5f;
    LeaderLine->SetWorldLocation(MidPoint);

    FRotator Rot = FRotationMatrix::MakeFromZ(Direction.GetSafeNormal()).Rotator();
    LeaderLine->SetWorldRotation(Rot);

    float MeshHeight = 100.f;
    LeaderLine->SetWorldScale3D(FVector(0.02f, 0.02f, Length / MeshHeight));
    
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

    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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

