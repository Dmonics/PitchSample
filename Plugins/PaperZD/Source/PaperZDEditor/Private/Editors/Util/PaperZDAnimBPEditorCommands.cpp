// Fill out your copyright notice in the Description page of Project Settings.

#include "PaperZDAnimBPEditorCommands.h"


#define LOCTEXT_NAMESPACE "PaperZDAnimBPEditorCommands"

/** UI_COMMAND takes long for the compile to optimize */
PRAGMA_DISABLE_OPTIMIZATION
void FPaperZDAnimBPEditorCommands::RegisterCommands()
{
	//AnimSequence commands
	UI_COMMAND(CreateAnimSequence, "New Animation Sequence", "Creates a new AnimSequence and registers it to this AnimBP for further use", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(DuplicateAnimSequence, "Duplicate", "Duplicates an AnimSequence with all the notifies and info copied", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(DeleteAnimSequence, "Delete", "Deletes an AnimSequence, unregistering from the Animation Blueprint", EUserInterfaceActionType::Button, FInputChord(EKeys::Platform_Delete));
	UI_COMMAND(RenameAnimSequence, "Rename", "Renames an AnimSequence, validating that the name isn't in use right now", EUserInterfaceActionType::Button, FInputChord(EKeys::F2));
}

PRAGMA_ENABLE_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
