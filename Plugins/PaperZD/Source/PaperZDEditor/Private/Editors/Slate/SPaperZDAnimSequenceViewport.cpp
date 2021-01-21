// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "SPaperZDAnimSequenceViewport.h"
#include "PaperZDEditor.h"
#include "AnimNodes/PaperZDAnimNode_State.h"

//Slate
#include  "Widgets/SNullWidget.h"

void SPaperZDAnimSequenceViewport::Construct(const FArguments& InArgs)
{
	AnimSequenceBeingEdited = InArgs._AnimSequenceBeingEdited;

	SEditorViewport::Construct(SEditorViewport::FArguments());
}

void SPaperZDAnimSequenceViewport::BindCommands()
{
	SEditorViewport::BindCommands();

	//@TODO: possible commands as: Show Grid, Show collisions, etc
}

TSharedRef<FEditorViewportClient> SPaperZDAnimSequenceViewport::MakeEditorViewportClient()
{
	EditorViewportClient = MakeShareable(new FPaperZDAnimSequenceViewportClient(AnimSequenceBeingEdited));

	return EditorViewportClient.ToSharedRef();
}

TSharedPtr<SWidget> SPaperZDAnimSequenceViewport::MakeViewportToolbar()
{
	return SNew(SVerticalBox);
}

EVisibility SPaperZDAnimSequenceViewport::GetTransformToolbarVisibility() const
{
	return EVisibility::Visible;
}

void SPaperZDAnimSequenceViewport::OnFocusViewportToSelection()
{
	//@TODO: add better focus on selecting
	//EditorViewportClient->RequestFocusOnSelection(/*bInstant=*/ false);
}

TSharedRef<class SEditorViewport> SPaperZDAnimSequenceViewport::GetViewportWidget()
{
	return SharedThis(this);
}

TSharedPtr<FExtender> SPaperZDAnimSequenceViewport::GetExtenders() const
{
	TSharedPtr<FExtender> Result(MakeShareable(new FExtender));
	return Result;
}

void SPaperZDAnimSequenceViewport::OnFloatingButtonClicked()
{
}
