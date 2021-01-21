// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "SPaperZDAnimGraphNode_State.h"
#include "PaperZDEditor.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SToolTip.h"
#include "Kismet2/BlueprintEditorUtils.h"
//#include "IDocumentation.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"

//#include "SGraphPreviewer.h"

//ZD
#include "PaperZDAnimGraphNode_State.h"
#include "PaperZDAnimGraph.h"

/////////////////////////////////////////////////////
// SPaperZDStateNodeOutputPin

class SPaperZDStateNodeOutputPin : public SGraphPin
{
public:
	SLATE_BEGIN_ARGS(SPaperZDStateNodeOutputPin) {}
	SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, UEdGraphPin* InPin);
protected:
	// Begin SGraphPin interface
	virtual TSharedRef<SWidget>	GetDefaultValueWidget() override;
	// End SGraphPin interface

	const FSlateBrush* GetPinBorder() const;
};

void SPaperZDStateNodeOutputPin::Construct(const FArguments& InArgs, UEdGraphPin* InPin)
{
	this->SetCursor(EMouseCursor::Default);

	bShowLabel = true;

	GraphPinObj = InPin;
	check(GraphPinObj != NULL);

	const UEdGraphSchema* Schema = GraphPinObj->GetSchema();
	check(Schema);

	// Set up a hover for pins that is tinted the color of the pin.
	SBorder::Construct(SBorder::FArguments()
		.BorderImage(this, &SPaperZDStateNodeOutputPin::GetPinBorder)
		.BorderBackgroundColor(this, &SPaperZDStateNodeOutputPin::GetPinColor)
		.OnMouseButtonDown(this, &SPaperZDStateNodeOutputPin::OnPinMouseDown)
		.Cursor(this, &SPaperZDStateNodeOutputPin::GetPinCursor)
	);
}

TSharedRef<SWidget>	SPaperZDStateNodeOutputPin::GetDefaultValueWidget()
{
	return SNew(STextBlock);
}

const FSlateBrush* SPaperZDStateNodeOutputPin::GetPinBorder() const
{
	return (IsHovered())
		? FEditorStyle::GetBrush(TEXT("Graph.StateNode.Pin.BackgroundHovered"))
		: FEditorStyle::GetBrush(TEXT("Graph.StateNode.Pin.Background"));
}

/////////////////////////////////////////////////////
// SPaperZDAnimGraphNode_State

void SPaperZDAnimGraphNode_State::Construct(const FArguments& InArgs, UPaperZDAnimGraphNode_State* InNode)
{
	this->GraphNode = InNode;

	this->SetCursor(EMouseCursor::CardinalCross);

	this->UpdateGraphNode();
}


void SPaperZDAnimGraphNode_State::GetStateInfoPopup(UEdGraphNode* GraphNode, TArray<FGraphInformationPopupInfo>& Popups)
{
	//@TODO: Add state info when Debug data is added
}

void SPaperZDAnimGraphNode_State::GetNodeInfoPopups(FNodeInfoContext* Context, TArray<FGraphInformationPopupInfo>& Popups) const
{
	GetStateInfoPopup(GraphNode, Popups);
}

FSlateColor SPaperZDAnimGraphNode_State::GetBorderBackgroundColor() const
{
	FLinearColor InactiveStateColor(0.08f, 0.08f, 0.08f);
	FLinearColor ActiveStateColorDim(0.4f, 0.3f, 0.15f);
	FLinearColor ActiveStateColorBright(1.f, 0.6f, 0.35f);

	//@TODO: Use the other colors when Debug Data is available

	return InactiveStateColor;
}

void SPaperZDAnimGraphNode_State::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();

	// Reset variables that are going to be exposed, in case we are refreshing an already setup node.
	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	const FSlateBrush* NodeTypeIcon = GetNameIcon();

	FLinearColor TitleShadowColor(0.6f, 0.6f, 0.6f);
	TSharedPtr<SErrorText> ErrorText;
	TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);

	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);
	this->GetOrAddSlot(ENodeZone::Center)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("Graph.StateNode.Body"))
			.Padding(0)
			.BorderBackgroundColor(this, &SPaperZDAnimGraphNode_State::GetBorderBackgroundColor)
			[
				SNew(SOverlay)

				// PIN AREA
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SAssignNew(RightNodeBox, SVerticalBox)
				]

				// STATE NAME AREA
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.Padding(10.0f)
				[
					SNew(SBorder)
					.BorderImage(FEditorStyle::GetBrush("Graph.StateNode.ColorSpill"))
					.BorderBackgroundColor(TitleShadowColor)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.Visibility(EVisibility::SelfHitTestInvisible)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							// POPUP ERROR MESSAGE
							SAssignNew(ErrorText, SErrorText)
							.BackgroundColor(this, &SPaperZDAnimGraphNode_State::GetErrorColor)
							.ToolTipText(this, &SPaperZDAnimGraphNode_State::GetErrorMsgToolTip)
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SNew(SImage)
							.Image(NodeTypeIcon)
						]
					
						+ SHorizontalBox::Slot()
						.Padding(FMargin(4.0f, 0.0f, 4.0f, 0.0f))
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								SAssignNew(InlineEditableText, SInlineEditableTextBlock)
								.Style(FEditorStyle::Get(), "Graph.StateNode.NodeTitleInlineEditableText")
								.Text(NodeTitle.Get(), &SNodeTitle::GetHeadTitle)
								.OnVerifyTextChanged(this, &SPaperZDAnimGraphNode_State::OnVerifyNameTextChanged)
								.OnTextCommitted(this, &SPaperZDAnimGraphNode_State::OnNameTextCommited)
								.IsReadOnly(this, &SPaperZDAnimGraphNode_State::IsNameReadOnly)
								.IsSelected(this, &SPaperZDAnimGraphNode_State::IsSelectedExclusively)
							]
							
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								NodeTitle.ToSharedRef()
							]
						]
					]
				]
			]
		];

	ErrorReporting = ErrorText;
	ErrorReporting->SetError(ErrorMsg);
	CreatePinWidgets();
}

void SPaperZDAnimGraphNode_State::CreatePinWidgets()
{
	UPaperZDAnimGraphNode_State* StateNode = CastChecked<UPaperZDAnimGraphNode_State>(GraphNode);

	UEdGraphPin* CurPin = StateNode->GetOutputPin();
	if (!CurPin->bHidden)
	{
		TSharedPtr<SGraphPin> NewPin = SNew(SPaperZDStateNodeOutputPin, CurPin);
		AddPin(NewPin.ToSharedRef());
	}
}

void SPaperZDAnimGraphNode_State::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
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

const FSlateBrush* SPaperZDAnimGraphNode_State::GetNameIcon() const
{
	return FEditorStyle::GetBrush(TEXT("Graph.StateNode.Icon"));
}