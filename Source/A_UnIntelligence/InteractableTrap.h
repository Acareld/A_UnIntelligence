// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "GameplayTagContainer.h"
#include "InteractableTrap.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

UCLASS(BlueprintType)
class UTrapDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly) FText DisplayName;
    UPROPERTY(EditDefaultsOnly) TObjectPtr<UAnimMontage> InteractMontage;
    UPROPERTY(EditDefaultsOnly) TObjectPtr<UAnimationAsset> TrapMeshAnim;
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

    UPROPERTY(EditInstanceOnly, Category = "ItemType")
    FGameplayTagContainer AcceptedItems;

protected:
    UPROPERTY(VisibleAnywhere)
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Mesh;

    UPROPERTY(VisibleAnywhere)
    UBoxComponent* InteractionVolume;

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
    UPROPERTY(EditAnywhere, Category = "Trap|Fall")
    float FallDuration = 0.8f;

    UPROPERTY(EditAnywhere, Category = "Trap|Fall")
    FRotator FallDelta = FRotator(0.f, 0.f, 90.f); 

    bool bIsFalling = false;
    float FallElapsed = 0.f;
    FRotator FallStartRot;
    FRotator FallTargetRot;
    bool bDidSnap = false;

    FVector TextWidgetDefaultPos;

    FTimerHandle FallTickTimer;
    FTimerHandle FallFinishedTimer;
    FTimerHandle RespawnDelayTimer;

    FVector LastViableAnchor;
    FVector TargetWidgetWorldLocation;
    FVector DesiredAnchor;
    TArray<FVector> Anchors;
    int32 TargetAnchorIndex;
    int32 CurrentAnchorIndex;
    int32 DefaultAnchorIndex = 0;

    bool bNeedTextSwitch = false;
    bool bOverlap = false;
    bool bIsSwitching = false;

    TWeakObjectPtr<AController> PendingController;

    void BeginFallOver(AController* ControllerToRespawn);
    void UpdateFall();
    void FinishFall();
    void DoDelayedRespawn();
    void CalculateTextAnchorPoints();
    int32 PickViableTextAnchor(int32 Current, FVector CamLoc);



};
