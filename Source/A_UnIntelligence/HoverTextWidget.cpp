// Fill out your copyright notice in the Description page of Project Settings.


#include "HoverTextWidget.h"
#include "Components/TextBlock.h"

void UHoverTextWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UHoverTextWidget::SetTrapName(const FText& NewName)
{
    if (TrapNameText)
    {
        TrapNameText->SetText(NewName);

    }
}