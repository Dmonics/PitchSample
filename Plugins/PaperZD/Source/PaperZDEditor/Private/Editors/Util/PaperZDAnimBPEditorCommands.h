// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorStyleSet.h"
#include "Framework/Commands/Commands.h"

/**
 * 
 */
class FPaperZDAnimBPEditorCommands : public TCommands<FPaperZDAnimBPEditorCommands>
{
public:

	FPaperZDAnimBPEditorCommands()
		: TCommands<FPaperZDAnimBPEditorCommands>(TEXT("PaperZDAnimBPEditor"), NSLOCTEXT("PaperZDContexts", "PaperZDAnimBPEditor", "PaperZD AnimBP Editor"), NAME_None, FEditorStyle::GetStyleSetName())
	{
	}

	virtual void RegisterCommands() override;

	//AnimSequence Related
	TSharedPtr<FUICommandInfo> CreateAnimSequence;
	TSharedPtr<FUICommandInfo> DuplicateAnimSequence;
	TSharedPtr<FUICommandInfo> DeleteAnimSequence;
	TSharedPtr<FUICommandInfo> RenameAnimSequence;
};
