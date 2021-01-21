// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "PaperZDAnimGraphNode_State.h"
#include "PaperZDEditor.h"
#include "AnimNodes/PaperZDAnimNode_State.h"
#include "Graphs/Nodes/PaperZDAnimGraphNode_Transition.h"
#include "AnimSequences/PaperZDAnimSequence.h"
#include "EdGraph/EdGraph.h"

//Slate related
#include "Slate/SPaperZDAnimGraphNode_State.h"
#include "Slate/SPaperZDAnimGraphNode_Transition.h"
#include "Slate/SPaperZDAnimGraphNode_Root.h"

//Kismet2
#include "Kismet2/Kismet2NameValidators.h"
#include "Kismet2/KismetEditorUtilities.h"

//////////////////////////////////////////////////////////////////////////
//// Name Validator
//////////////////////////////////////////////////////////////////////////
class FPaperZDStateNodeNameValidator : public FStringSetNameValidator
{
public:
	FPaperZDStateNodeNameValidator(const UPaperZDAnimGraphNode_State* InStateNode)
		: FStringSetNameValidator(FString())
	{
		TArray<UPaperZDAnimGraphNode_State*> Nodes;
		InStateNode->GetGraph()->GetNodesOfClass(Nodes);

		for (auto NodeIt = Nodes.CreateIterator(); NodeIt; ++NodeIt)
		{
			auto Node = *NodeIt;
			if (Node != InStateNode)
			{
				Names.Add(Node->GetStateName());
			}
		}
	}
};

//////////////////////////////////////////////////////////////////////////
//// PaperZD Anim Graph Node
//////////////////////////////////////////////////////////////////////////

UPaperZDAnimGraphNode_State::UPaperZDAnimGraphNode_State(const FObjectInitializer& ObjectInitializer)
	: Super()
{
	Name = FText::FromString(TEXT("State"));
	bCanRenameNode = true;
	bShouldLoop = true;
}

void UPaperZDAnimGraphNode_State::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	//Ensure the flipbook is given to the AnimNode
	UPaperZDAnimNode_State *StateNode = CastChecked<UPaperZDAnimNode_State>(AnimNode);
	StateNode->bShouldLoop = bShouldLoop;
}

FText UPaperZDAnimGraphNode_State::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return Name;
}

void UPaperZDAnimGraphNode_State::OnRenameNode(const FString & NewName)
{
	Name = FText::FromString(NewName);
}

void UPaperZDAnimGraphNode_State::AllocateDefaultPins()
{
	FCreatePinParams PinParams;
	UEdGraphPin* Inputs = CreatePin(EGPD_Input, TEXT("Transition"), TEXT(""), NULL, TEXT("In"), PinParams);
	UEdGraphPin* Outputs = CreatePin(EGPD_Output, TEXT("Transition"), TEXT(""), NULL, TEXT("Out"), PinParams);
}

void UPaperZDAnimGraphNode_State::AutowireNewNode(UEdGraphPin* FromPin)
{
	Super::AutowireNewNode(FromPin);

	if (FromPin != NULL)
	{
		if (GetSchema()->TryCreateConnection(FromPin, GetInputPin()))
		{
			FromPin->GetOwningNode()->NodeConnectionListChanged();
		}
	}
}

TSharedPtr<SGraphNode> UPaperZDAnimGraphNode_State::CreateVisualWidget()
{
	return SNew(SPaperZDAnimGraphNode_State, this);
}

void UPaperZDAnimGraphNode_State::JumpToDefinition() const
{
	FKismetEditorUtilities::BringKismetToFocusAttentionOnObject(GetJumpTargetForDoubleClick());
}

TArray<UPaperZDAnimGraphNode_Transition *> UPaperZDAnimGraphNode_State::GetTransitions()
{
	TArray<UPaperZDAnimGraphNode_Transition *> Transitions;

	for (auto pin : GetOutputPin()->LinkedTo)
	{
		UPaperZDAnimGraphNode_Transition *TransitionNode = Cast<UPaperZDAnimGraphNode_Transition>(pin->GetOwningNode());

		if (TransitionNode)
			Transitions.Add(TransitionNode);
	}

	return Transitions;
}

UObject* UPaperZDAnimGraphNode_State::GetJumpTargetForDoubleClick() const
{
	//Must return an object that has a Blueprint Outer or in the Blueprint Outer Hierarchy, we return this same node and the editor will open the AnimNode that has different ownership
	return const_cast<UPaperZDAnimGraphNode_State*>(this);
}

TSharedPtr<INameValidatorInterface> UPaperZDAnimGraphNode_State::MakeNameValidator() const
{
	return MakeShareable(new FPaperZDStateNodeNameValidator(this));
}
