// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "Slate/SPaperZDAnimGraphNode_Root.h"
#include "PaperZDEditor.h"
#include "PaperZDAnimGraphNode_Root.h"
#include "Widgets/SBoxPanel.h"
#include "SGraphPin.h"

/////////////////////////////////////////////////////
// SGraphNodeAnimStateEntry

void SPaperZDAnimGraphNode_Root::Construct(const FArguments& InArgs, UPaperZDAnimGraphNode_Root* InNode)
{
	this->GraphNode = InNode;

	this->SetCursor(EMouseCursor::CardinalCross);

	this->UpdateGraphNode();
}

void SPaperZDAnimGraphNode_Root::GetNodeInfoPopups(FNodeInfoContext* Context, TArray<FGraphInformationPopupInfo>& Popups) const
{

}

FSlateColor SPaperZDAnimGraphNode_Root::GetBorderBackgroundColor() const
{
	FLinearColor InactiveStateColor(0.08f, 0.08f, 0.08f);
	FLinearColor ActiveStateColorDim(0.4f, 0.3f, 0.15f);
	FLinearColor ActiveStateColorBright(1.f, 0.6f, 0.35f);

	return InactiveStateColor;
}

void SPaperZDAnimGraphNode_Root::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();

	// Reset variables that are going to be exposed, in case we are refreshing an already setup node.
	RightNodeBox.Reset();
	LeftNodeBox.Reset();


	FLinearColor TitleShadowColor(0.6f, 0.6f, 0.6f);

	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);
	this->GetOrAddSlot(ENodeZone::Center)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("Graph.StateNode.Body"))
		.Padding(0)
		.BorderBackgroundColor(this, &SPaperZDAnimGraphNode_Root::GetBorderBackgroundColor)
		[
			SNew(SOverlay)

			// PIN AREA
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.Padding(10.0f)
		[
			SAssignNew(RightNodeBox, SVerticalBox)
		]
		]
		];

	CreatePinWidgets();
}

void SPaperZDAnimGraphNode_Root::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	PinToAdd->SetOwner(SharedThis(this));
	RightNodeBox->AddSlot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.FillHeight(1.0f)
		[
			PinToAdd
		];
	OutputPins.Add(PinToAdd);
}

FText SPaperZDAnimGraphNode_Root::GetPreviewCornerText() const
{
	return NSLOCTEXT("SPaperZDAnimGraphNode_Root", "CornerTextDescription", "Entry point for state machine");
}
