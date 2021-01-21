// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "Slate/SPaperZDAnimGraphNode_Jump.h"
#include "PaperZDEditor.h"
#include "PaperZDAnimGraphNode_Jump.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "SGraphPin.h"

/////////////////////////////////////////////////////
// SGraphNodeAnimNode_Jump

void SPaperZDAnimGraphNode_Jump::Construct(const FArguments& InArgs, UPaperZDAnimGraphNode_Jump* InNode)
{
	GraphNode = InNode;
	SetCursor(EMouseCursor::CardinalCross);
	UpdateGraphNode();
}

FSlateColor SPaperZDAnimGraphNode_Jump::GetBorderBackgroundColor() const
{
	FLinearColor InactiveStateColor(0.08f, 0.08f, 0.08f);
	FLinearColor ActiveStateColorDim(0.4f, 0.3f, 0.15f);
	FLinearColor ActiveStateColorBright(1.f, 0.6f, 0.35f);

	return InactiveStateColor;
}

void SPaperZDAnimGraphNode_Jump::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();

	// Reset variables that are going to be exposed, in case we are refreshing an already setup node.
	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	//const FSlateBrush* NodeTypeIcon = GetNameIcon();
	FLinearColor TitleShadowColor(0.6f, 0.6f, 0.6f);
	TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);

	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);
	this->GetOrAddSlot(ENodeZone::Center)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("Graph.StateNode.Body"))
			.Padding(0)
			.BorderBackgroundColor(this, &SPaperZDAnimGraphNode_Jump::GetBorderBackgroundColor)
			[
				SNew(SOverlay)
				+SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.Padding(10.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(FMargin(4.0f, 0.0f, 4.0f, 0.0f))
					.AutoWidth()
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SAssignNew(InlineEditableText, SInlineEditableTextBlock)
							.Style(FEditorStyle::Get(), "Graph.StateNode.NodeTitleInlineEditableText")
							.Text(NodeTitle.Get(), &SNodeTitle::GetHeadTitle)
							.OnVerifyTextChanged(this, &SPaperZDAnimGraphNode_Jump::OnVerifyNameTextChanged)
							.OnTextCommitted(this, &SPaperZDAnimGraphNode_Jump::OnNameTextCommited)
							.IsReadOnly(this, &SPaperZDAnimGraphNode_Jump::IsNameReadOnly)
							.IsSelected(this, &SPaperZDAnimGraphNode_Jump::IsSelectedExclusively)
						]
						
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							NodeTitle.ToSharedRef()
						]
					]
					
					+SHorizontalBox::Slot()
					.AutoWidth()
					[
						SAssignNew(RightNodeBox, SVerticalBox)
					]
				]
			]
		];

	CreatePinWidgets();
}

void SPaperZDAnimGraphNode_Jump::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
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

FText SPaperZDAnimGraphNode_Jump::GetPreviewCornerText() const
{
	return NSLOCTEXT("SPaperZDAnimGraphNode_Jump", "CornerTextDescription", "Extra entry point of the state machine that can be jumped to");
}
