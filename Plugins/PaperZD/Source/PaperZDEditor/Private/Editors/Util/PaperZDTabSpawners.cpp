// Fill out your copyright notice in the Description page of Project Settings.

#include "PaperZDTabSpawners.h"
#include "Slate/SPaperZDMySequences.h"
#include "AnimSequences/PaperZDAnimSequence.h"
#include "EditorStyleSet.h"
#include "SScrubControlPanel.h"
#include "SPaperZDAnimNotifyPanel.h"
#include "SPaperZDAnimSequenceViewport.h"
#include "Editors/PaperZDAnimSequenceViewportClient.h"

#define LOCTEXT_NAMESPACE "PaperZDTabs"
const FName FPaperZDTabs::MySequencesTabID("MySequencesTab");
const FName FPaperZDTabs::SequenceDocumentTabID("SequencesDocumentTab");

/************************************************************************/
/* MySequences                                                          */
/************************************************************************/
FPaperZDMySequencesTabSummoner::FPaperZDMySequencesTabSummoner(TSharedPtr<FPaperZDAnimBPEditor> InHostingApp) : FWorkflowTabFactory(FPaperZDTabs::MySequencesTabID, InHostingApp), AnimBPEditor(InHostingApp)
{
	TabLabel = LOCTEXT("MySequencesTab", "My Sequences");
}

TSharedRef<SWidget> FPaperZDMySequencesTabSummoner::CreateTabBody(const FWorkflowTabSpawnInfo& Info)const
{
	return SNew(SPaperZDMySequences, AnimBPEditor);
}


FText FPaperZDMySequencesTabSummoner::GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const
{
	return LOCTEXT("MySequencesTab_TooltipText", "Manage the animation sequences to use on this AnimBP");
}

/************************************************************************/
/* AnimSequenceDocumentTabSummoner                                      */
/************************************************************************/
TAttribute<FText> FAnimSequenceDocumentTabSummoner::ConstructTabNameForObject(UPaperZDAnimSequence* DocumentID) const
{
	return TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &FAnimSequenceDocumentTabSummoner::GetSequenceName, TWeakObjectPtr<UPaperZDAnimSequence>(DocumentID)));
}

TSharedRef<SWidget> FAnimSequenceDocumentTabSummoner::CreateTabBodyForObject(const FWorkflowTabSpawnInfo& Info, UPaperZDAnimSequence* DocumentID) const
{
	ViewportPtr = SNew(SPaperZDAnimSequenceViewport).AnimSequenceBeingEdited(DocumentID);
	TSharedRef<FPaperZDAnimSequenceViewportClient> ViewportClient = ViewportPtr->GetEditorViewportClient();
	ViewportClientPtr = TSharedPtr<FPaperZDAnimSequenceViewportClient>(ViewportClient);

	TSharedRef<SWidget> ScrubControl = SNew(SScrubControlPanel)
		.IsEnabled(true)
		.Value(ViewportClient, &FPaperZDAnimSequenceViewportClient::GetPlaybackPosition)
		.NumOfKeys(ViewportClient, &FPaperZDAnimSequenceViewportClient::GetTotalFrameCountPlusOne)
		.SequenceLength(ViewportClient, &FPaperZDAnimSequenceViewportClient::GetTotalSequenceLength)
		.OnValueChanged(ViewportClient, &FPaperZDAnimSequenceViewportClient::SetPlaybackPosition)
		.OnClickedForwardPlay(ViewportClient, &FPaperZDAnimSequenceViewportClient::OnClick_Forward)
		.OnClickedForwardStep(ViewportClient, &FPaperZDAnimSequenceViewportClient::OnClick_Forward_Step)
		.OnClickedForwardEnd(ViewportClient, &FPaperZDAnimSequenceViewportClient::OnClick_Forward_End)
		.OnClickedBackwardPlay(ViewportClient, &FPaperZDAnimSequenceViewportClient::OnClick_Backward)
		.OnClickedBackwardStep(ViewportClient, &FPaperZDAnimSequenceViewportClient::OnClick_Backward_Step)
		.OnClickedBackwardEnd(ViewportClient, &FPaperZDAnimSequenceViewportClient::OnClick_Backward_End)
		.OnClickedToggleLoop(ViewportClient, &FPaperZDAnimSequenceViewportClient::OnClick_ToggleLoop)
		.OnGetLooping(ViewportClient, &FPaperZDAnimSequenceViewportClient::IsLooping)
		.OnGetPlaybackMode(ViewportClient, &FPaperZDAnimSequenceViewportClient::GetPlaybackMode)
		.ViewInputMin(ViewportClient, &FPaperZDAnimSequenceViewportClient::GetViewRangeMin)
		.ViewInputMax(ViewportClient, &FPaperZDAnimSequenceViewportClient::GetViewRangeMax)
		.OnSetInputViewRange(ViewportClient, &FPaperZDAnimSequenceViewportClient::SetViewRange)
		.bAllowZoom(true)
		.IsRealtimeStreamingMode(false);

	TSharedRef<SVerticalBox> VerticalBox = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		[
			ViewportPtr.ToSharedRef()
		]

	+ SVerticalBox::Slot()
		.Padding(0, 8, 0, 0)
		.AutoHeight()
		[
			SAssignNew(NotifyPanelPtr, SPaperZDAnimNotifyPanel)
			.WidgetWidth(200.0f) //@TODO: HARDCODED, maybe modify it
			.OnGetPlaybackValue(ViewportClient, &FPaperZDAnimSequenceViewportClient::GetPlaybackPosition)
			.OnGetNumKeys(ViewportClient, &FPaperZDAnimSequenceViewportClient::GetTotalFrameCountPlusOne)
			.OnGetSequenceLength(ViewportClient, &FPaperZDAnimSequenceViewportClient::GetTotalSequenceLength)
			.ViewInputMin(ViewportClient, &FPaperZDAnimSequenceViewportClient::GetViewRangeMin)
			.ViewInputMax(ViewportClient, &FPaperZDAnimSequenceViewportClient::GetViewRangeMax)
			.OnSetViewRange(ViewportClient, &FPaperZDAnimSequenceViewportClient::SetViewRange)
			.OnNotifySelectionChanged(Editor.Pin().ToSharedRef(), &FPaperZDAnimBPEditor::OnNotifySelectionChanged)
			.AnimSequence(DocumentID)
			//.AnimBP(AnimBPBeingEdited)
		]

	+SVerticalBox::Slot()
		.Padding(0, 8, 0, 0)
		.AutoHeight()
		[
			ScrubControl
		];

	return VerticalBox;
}

const FSlateBrush* FAnimSequenceDocumentTabSummoner::GetTabIconForObject(const FWorkflowTabSpawnInfo& Info, UPaperZDAnimSequence* DocumentID) const
{
	return FEditorStyle::GetBrush("GraphEditor.Timeline_16x");
}

FText FAnimSequenceDocumentTabSummoner::GetSequenceName(TWeakObjectPtr<UPaperZDAnimSequence> Sequence) const
{
	return Sequence.IsValid() ? FText::FromName(Sequence->GetSequenceName()) : FText::FromString("INVALID");
}

#undef LOCTEXT_NAMESPACE