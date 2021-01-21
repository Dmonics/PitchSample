// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "SEditorViewport.h"
#include "SCommonEditorViewportToolbarBase.h"
#include "PaperZDAnimSequenceViewportClient.h"

class SPaperZDAnimSequenceViewport : public SEditorViewport, public ICommonEditorViewportToolbarInfoProvider
{
public:
	SLATE_BEGIN_ARGS(SPaperZDAnimSequenceViewport)
		: _AnimSequenceBeingEdited((class UPaperZDAnimSequence*)nullptr)
	{}

	SLATE_ATTRIBUTE(class UPaperZDAnimSequence*, AnimSequenceBeingEdited)

		SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	// SEditorViewport interface
	virtual void BindCommands() override;
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
	virtual TSharedPtr<SWidget> MakeViewportToolbar() override;
	virtual EVisibility GetTransformToolbarVisibility() const override;
	virtual void OnFocusViewportToSelection() override;
	// End of SEditorViewport interface

	// ICommonEditorViewportToolbarInfoProvider interface
	virtual TSharedRef<class SEditorViewport> GetViewportWidget() override;
	virtual TSharedPtr<FExtender> GetExtenders() const override;
	virtual void OnFloatingButtonClicked() override;
	// End of ICommonEditorViewportToolbarInfoProvider interface

	TSharedRef<FPaperZDAnimSequenceViewportClient> GetEditorViewportClient()
	{
		check(EditorViewportClient.IsValid());

		return EditorViewportClient.ToSharedRef();
	}

private:
	TAttribute<class UPaperZDAnimSequence*> AnimSequenceBeingEdited;

	// Viewport client
	TSharedPtr<FPaperZDAnimSequenceViewportClient> EditorViewportClient;
};