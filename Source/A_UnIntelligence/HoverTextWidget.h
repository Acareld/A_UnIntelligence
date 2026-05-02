// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HoverTextWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class A_UNINTELLIGENCE_API UHoverTextWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetTrapName(const FText& NewName);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TrapNameText;
};
