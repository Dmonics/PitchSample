// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

struct FChangeModeInfo {

	FChangeModeInfo() {}
	FChangeModeInfo(FString InIdentifier, FName InDisplayName, FText InTooltipText, FName InBrushName) : Identifier(InIdentifier), DisplayName(InDisplayName), TooltipText(InTooltipText), BrushName(InBrushName)
	{
	}

	FString Identifier;
	FName DisplayName;
	FText TooltipText;
	FName BrushName;
};

DECLARE_DELEGATE_OneParam(FOnModeSelected, FString)
DECLARE_DELEGATE_RetVal(FString, FOnGetCurrentSectionIdentifier)
class SPaperZDModeSelectorWidget: public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPaperZDModeSelectorWidget) 
	{
	}
	SLATE_EVENT(FOnModeSelected, OnModeSelected)
	SLATE_EVENT(FOnGetCurrentSectionIdentifier, OnGetCurrentSectionIdentifier)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TArray<FChangeModeInfo> ChangeModeInfo);
	void HandleSectionSelected(FString Identifier);
	FString GetCurrentSectionIdentifier() { return CurrentSectionIdentifier; }
	
private:
	FOnModeSelected OnModeSelected;
	FString CurrentSectionIdentifier;
};
