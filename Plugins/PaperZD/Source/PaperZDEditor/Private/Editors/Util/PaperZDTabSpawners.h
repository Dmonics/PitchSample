// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperZDAnimBPEditor.h"
#include "WorkflowOrientedApp/WorkflowTabFactory.h"
#include "WorkflowOrientedApp/WorkflowUObjectDocuments.h"

struct FPaperZDTabs {
	static const FName MySequencesTabID;
	static const FName SequenceDocumentTabID;

private:
	FPaperZDTabs() {}
};

/************************************************************************/
/* MySequences                                                          */
/************************************************************************/
class FPaperZDMySequencesTabSummoner: public FWorkflowTabFactory
{
public:
	FPaperZDMySequencesTabSummoner(TSharedPtr<FPaperZDAnimBPEditor> InHostingApp);

	//FWorkflowTabFactory
	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;
	virtual FText GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const override;
	//End FWorkflowTabFactory

	TWeakPtr<FPaperZDAnimBPEditor> AnimBPEditor;
};

/************************************************************************/
/* AnimSequenceDocumentTabSummoner                                      */
/************************************************************************/
class FAnimSequenceDocumentTabSummoner : public FDocumentTabFactoryForObjects<UPaperZDAnimSequence>
{
public:
	class UPaperZDAnimBP* AnimBPBeingEdited;

protected:
	TWeakPtr<FPaperZDAnimBPEditor> Editor;
	mutable TSharedPtr<class FPaperZDAnimSequenceViewportClient> ViewportClientPtr;
	mutable TSharedPtr<class SPaperZDAnimSequenceViewport> ViewportPtr;
	mutable TSharedPtr<class SPaperZDAnimNotifyPanel> NotifyPanelPtr;
	
public:
	FAnimSequenceDocumentTabSummoner(TSharedPtr<FPaperZDAnimBPEditor> InHostingApp, class UPaperZDAnimBP *InAnimBP)
		: FDocumentTabFactoryForObjects<UPaperZDAnimSequence>(FPaperZDTabs::SequenceDocumentTabID, InHostingApp)
		, AnimBPBeingEdited(InAnimBP)
		, Editor(InHostingApp)
	{
	}

	// ~FDocumentTabFactoryForObjects
	virtual TAttribute<FText> ConstructTabNameForObject(UPaperZDAnimSequence* DocumentID) const override;
	virtual TSharedRef<SWidget> CreateTabBodyForObject(const FWorkflowTabSpawnInfo& Info, UPaperZDAnimSequence* DocumentID) const override;
	virtual const FSlateBrush* GetTabIconForObject(const FWorkflowTabSpawnInfo& Info, UPaperZDAnimSequence* DocumentID) const override;
	// ~FDocumentTabFactoryForObjects

	FText GetSequenceName(TWeakObjectPtr<UPaperZDAnimSequence> Sequence) const;
};
