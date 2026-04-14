// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "GameplayTagContainer.h"
#include "LevelSequence.h"
#include "CharacterController.h"
#include "Components/BoxComponent.h"
#include "LevelSequencePlayer.h"

#include "InteractableTrap.generated.h"


class UStaticMeshComponent;
class UBoxComponent;
class UNiagaraSystem;

UENUM(BlueprintType)
enum class ETrapVisualType : uint8
{
    StaticMesh,
    SkeletalMesh,
    None
};

UENUM(BlueprintType)
enum class EAnimPlayOrder : uint8
{
    PlayerFirst,
    TrapFirst,
    Simultaneously
};



UCLASS(BlueprintType)
class A_UNINTELLIGENCE_API UTrapDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="General")
    bool IsTrap = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "General")
    FText DisplayName;

    // Animation
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
    EAnimPlayOrder PlayOrder = EAnimPlayOrder::PlayerFirst;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
    TObjectPtr<UAnimationAsset> PlayerAnim = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
    TObjectPtr<UAnimationAsset> TrapMeshAnim = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
    TObjectPtr<ULevelSequence> TrapSequence = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Timing")
    float RespawnDelay = 0.f;

    // Only used with EAnimPlayOrder::PlayerFirst, percentage of first anim played before starting second
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Timing", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "PlayOrder == EAnimPlayOrder::PlayerFirst", EditConditionHides))
    float DelayPercentage = 1.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Timing")
    bool bReverseAnimAfterDelay = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Timing", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0",
        EditCondition = "bReverseAnimAfterDelay", EditConditionHides))
    float ReverseDelayPercentage = 1.f;

    // Animation attach 
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Animation|Attachment")
    bool bAttachToSocket = false;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Animation|Attachment",
        meta = (EditCondition = "bAttachToSocket", EditConditionHides)
    )
    FName SocketName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Placement")
    FVector AnimationPosition;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Placement")
    FRotator AnimationRotation;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Placement")
    FVector ContinuePosition;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Placement")
    FRotator ContinueRotation;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Placement")
    FVector StartAnimationPosition = AnimationPosition;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Placement")
    float FinalPoseZOffset;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
    TObjectPtr<USoundBase> MetaSound = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float SoundDelay = 0.0f;

    // ----------------------------
    // VFX Settings
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Effects")
    bool bUseVFX = false;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Effects",
        meta = (EditCondition = "bUseVFX", EditConditionHides)
    )
    TObjectPtr<UNiagaraSystem> EffectSystem = nullptr;
    
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Effects|Timing",
        meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0",
            EditCondition = "bUseVFX", EditConditionHides)
    )
    float EffectDelayPercentage = 1.f;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Effects|Placement",
        meta = (EditCondition = "bUseVFX", EditConditionHides)
    )
    FVector EffectLocation = FVector(0.f, 0.f, 0.f);

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Effects|Placement",
        meta = (EditCondition = "bUseVFX", EditConditionHides)
    )
    FVector EffectBoxSize = FVector(0.f, 0.f, 0.f);

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Effects|Placement",
        meta = (EditCondition = "bUseVFX", EditConditionHides)
    )
    FRotator EffectRotation = FRotator(0.f, 0.f, 0.f);

    // ------------------------------


    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Activation")
    bool bActivateAfterSequence = false;

};

UCLASS()
class A_UNINTELLIGENCE_API AInteractableTrap : public AActor, public IInteractable
{
    GENERATED_BODY()

public:
    AInteractableTrap();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual EInteractionType GetInteractionType() const override;

    // Deprecated
    UPROPERTY(EditInstanceOnly, Category = "Trap")
    FGameplayTagContainer AcceptedItems;

protected:
    UPROPERTY(VisibleAnywhere)
    USceneComponent* RootSceneComponent;

    // DEPRECATED
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Mesh;

    UPROPERTY(VisibleAnywhere, Category = "Trap")
    TObjectPtr<UStaticMeshComponent> StaticMeshComp;

    UPROPERTY(VisibleAnywhere, Category = "Trap")
    TObjectPtr<USkeletalMeshComponent> SkeletalMeshComp;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UMeshComponent> ActiveMeshComp;

    UPROPERTY(EditInstanceOnly, Category = "Trap")
    ETrapVisualType VisualType = ETrapVisualType::StaticMesh;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UBoxComponent> InteractionVolume;

    FVector TopWorldCorners[5];

    
    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Trap")
    TObjectPtr<UTrapDefinition> TrapDef;

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Trap")
    TObjectPtr<AActor> ActorToActivate = nullptr;

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
    float LeaderLineOffsetY;

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
    float LeaderLineOffsetX;

    UPROPERTY(VisibleDefaultsOnly, Category = "UI")
    class UWidgetComponent* NameWidget;

    UPROPERTY(VisibleDefaultsOnly, Category = "UI")
    class UStaticMeshComponent* LeaderLineDown;
    UPROPERTY(VisibleDefaultsOnly, Category = "UI")
    class UStaticMeshComponent* LeaderLineSide;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trap")
    USkeletalMesh* FridgeSkeletalMesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Trap")
    UAnimationAsset* FridgeAnimationAsset;

    TWeakObjectPtr<APawn> OverlappingPawn;
    TWeakObjectPtr<APawn> AnimInstigatorPawn;

    FTimerHandle UIUpdateTimer;

    bool bUIVisible = true;

    void UpdateHoverUI();
    void SetHoverUIVisible(bool bVisible);

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void CalculateTopWorldCorners();

public:
    virtual void Interact_Implementation(APawn* InstigatorPawn) override;

    virtual bool CanPickup(AActor* ByActor) override;

    bool IsInteractable(AActor* Other);

private:
    FVector TextWidgetDefaultPos;

    FTimerHandle AnimationDelayTimer;
    FTimerHandle ReverseAnimTimer;
    FTimerHandle DelayedRespawnTimer;
    FTimerHandle VFXTimer;
    FTimerHandle OffsetTimer;
    FTimerHandle SoundTimer;
    FTimerHandle FreezerTimer;

    FVector LastViableAnchor;
    FVector TargetWidgetWorldLocation;
    FVector DesiredAnchor;
    TArray<FVector> Anchors;
    int32 TargetAnchorIndex;
    int32 CurrentAnchorIndex;
    int32 DefaultAnchorIndex = 0;

    // AnimData
    float MaxAnimLength = 0;
    TObjectPtr<UAnimSequence> TrapMeshAnim = nullptr;
    TObjectPtr<ULevelSequencePlayer> TrapSequencePlayer = nullptr;
    TObjectPtr<UAnimationAsset> PlayerAnim = nullptr;

    bool bNeedTextSwitch = false;
    bool bOverlap = false;
    bool bIsSwitching = false;
    bool bMoveToInteraction = false;

    TWeakObjectPtr<AController> PendingController;

    void CalculateTextAnchorPoints();
    int32 PickViableTextAnchor(int32 Current, FVector CamLoc);
    void PlayAnimations(APawn* Pawn);
    void RefreshActiveMesh();
    void PlayTrapAnimation(bool bReverse);
    void CollectAnimData();
    void FireVFX();
    void PlaySound();
    void SpawnFrozenPoseCopy(USkeletalMeshComponent* SourceMesh, FTransform SpawnTransform, UAnimationAsset* PoseAnim);
    void RespawnAfterFreezer();

    UFUNCTION()
    void ActivateOtherActor();

    UFUNCTION()
    void DelayedRespawn();
    bool GetActiveMeshLocalBounds(FVector& OutMin, FVector& OutMax) const
    {
        
        if (VisualType == ETrapVisualType::StaticMesh)
        {
            if (const UStaticMeshComponent* StaticComp = Cast<UStaticMeshComponent>(ActiveMeshComp))
            {
                StaticComp->GetLocalBounds(OutMin, OutMax);
                return true;
            }      
        }

        if (VisualType == ETrapVisualType::SkeletalMesh)
        {
            if (const USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>(ActiveMeshComp)) {
                const FBoxSphereBounds Bounds = SkelComp->GetLocalBounds();
                const FBox Box = Bounds.GetBox();
                OutMin = Box.Min;
                OutMax = Box.Max;
                return true;
            }
        }
        if (VisualType == ETrapVisualType::None)
        {
            const FBoxSphereBounds Bounds = InteractionVolume->GetLocalBounds();
            const FBox Box = Bounds.GetBox();
            OutMin = Box.Min;
            OutMax = Box.Max;
            return true;
        }

        OutMin = FVector::ZeroVector;
        OutMax = FVector::ZeroVector;
        return false;
    }



};
