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

    UPROPERTY(EditDefaultsOnly) 
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly)
    EAnimPlayOrder PlayOrder = EAnimPlayOrder::PlayerFirst;

    // Only used with EAnimPlayOrder::PlayerFirst, percentage of first anim played before starting second
    UPROPERTY(EditDefaultsOnly)
    float DelayPercentage = 1.f;

    UPROPERTY(EditDefaultsOnly)
    bool ReverseAnimAfterDelay = false;

    UPROPERTY(EditDefaultsOnly)
    float ReverseDelayPercentage = 1.f;

    UPROPERTY(EditDefaultsOnly)
    bool AttachToSocket = false;

    // ----------------------------
    // VFX Settings
    UPROPERTY(EditDefaultsOnly)
    bool UseVFX = false;

    UPROPERTY(EditDefaultsOnly)
    float EffectDelayPercentage = 1.f;

    UPROPERTY(EditDefaultsOnly)
    UNiagaraSystem* EffectSystem = nullptr;

    UPROPERTY(EditDefaultsOnly)
    FVector EffectLocation = FVector(0.f, 0.f, 0.f);

    UPROPERTY(EditDefaultsOnly)
    FVector EffectBoxSize = FVector(0.f, 0.f, 0.f);

    UPROPERTY(EditDefaultsOnly)
    FRotator EffectRotation = FRotator(0.f, 0.f, 0.f);

    // ------------------------------

    UPROPERTY(EditDefaultsOnly)
    FName SocketName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FVector AnimationPosition;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FRotator AnimationRotation;

    // not used rn
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<UAnimMontage> InteractMontage = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<UAnimationAsset> TrapMeshAnim = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<ULevelSequence> TrapSequence = nullptr;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<UAnimationAsset> PlayerAnim = nullptr;

   

    // add VFX/SFX/etc
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

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> StaticMeshComp;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USkeletalMeshComponent> SkeletalMeshComp;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UMeshComponent> ActiveMeshComp;

    UPROPERTY(EditInstanceOnly, Category = "Trap")
    ETrapVisualType VisualType = ETrapVisualType::StaticMesh;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UBoxComponent> InteractionVolume;

    FVector TopWorldCorners[5];

    

    UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
    TObjectPtr<UTrapDefinition> TrapDef;

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
    TObjectPtr<UAnimSequence> PlayerAnim = nullptr;

    bool bNeedTextSwitch = false;
    bool bOverlap = false;
    bool bIsSwitching = false;

    TWeakObjectPtr<AController> PendingController;

    void CalculateTextAnchorPoints();
    int32 PickViableTextAnchor(int32 Current, FVector CamLoc);
    void PlayAnimations(APawn* Pawn);
    void RefreshActiveMesh();
    void PlayTrapAnimation(bool bReverse);
    void CollectAnimData();
    void FireVFX();

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
