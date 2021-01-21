// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "PaperZDAnimGraphNode_Conduit.h"
#include "PaperZDEditor.h"
#include "AnimNodes/PaperZDAnimNode_Conduit.h"
#include "Graphs/Nodes/PaperZDAnimGraphNode_Transition.h"
#include "EdGraph/EdGraph.h"

//Slate related
#include "Slate/SPaperZDAnimGraphNode_Conduit.h"

//Kismet2
#include "Kismet2/Kismet2NameValidators.h"
#include "Kismet2/KismetEditorUtilities.h"

//////////////////////////////////////////////////////////////////////////
//// Name Validator
//////////////////////////////////////////////////////////////////////////
class FPaperZDConduitNodeNameValidator : public FStringSetNameValidator
{
public:
	FPaperZDConduitNodeNameValidator(const UPaperZDAnimGraphNode_Conduit* InStateNode)
		: FStringSetNameValidator(FString())
	{
		TArray<UPaperZDAnimGraphNode_Conduit*> Nodes;
		InStateNode->GetGraph()->GetNodesOfClass(Nodes);

		for (auto NodeIt = Nodes.CreateIterator(); NodeIt; ++NodeIt)
		{
			auto Node = *NodeIt;
			if (Node != InStateNode)
			{
				Names.Add(Node->GetNameAsString());
			}
		}
	}
};

//////////////////////////////////////////////////////////////////////////
//// PaperZD Anim Graph Node
//////////////////////////////////////////////////////////////////////////

UPaperZDAnimGraphNode_Conduit::UPaperZDAnimGraphNode_Conduit(const FObjectInitializer& ObjectInitializer)
	: Super()
{
	Name = FText::FromString(TEXT("Conduit"));
	bCanRenameNode = true;
}

FText UPaperZDAnimGraphNode_Conduit::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return Name;
}

void UPaperZDAnimGraphNode_Conduit::OnRenameNode(const FString & NewName)
{
	Name = FText::FromString(NewName);
}

void UPaperZDAnimGraphNode_Conduit::AllocateDefaultPins()
{
	FCreatePinParams PinParams;
	UEdGraphPin* Inputs = CreatePin(EGPD_Input, TEXT("Transition"), TEXT(""), NULL, TEXT("In"), PinParams);
	UEdGraphPin* Outputs = CreatePin(EGPD_Output, TEXT("Transition"), TEXT(""), NULL, TEXT("Out"), PinParams);
}

void UPaperZDAnimGraphNode_Conduit::AutowireNewNode(UEdGraphPin* FromPin)
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

TSharedPtr<SGraphNode> UPaperZDAnimGraphNode_Conduit::CreateVisualWidget()
{
	return SNew(SPaperZDAnimGraphNode_Conduit, this);
}

TArray<UPaperZDAnimGraphNode_Transition *> UPaperZDAnimGraphNode_Conduit::GetTransitions()
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

TSharedPtr<INameValidatorInterface> UPaperZDAnimGraphNode_Conduit::MakeNameValidator() const
{
	return MakeShareable(new FPaperZDConduitNodeNameValidator(this));
}
