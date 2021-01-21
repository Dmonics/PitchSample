// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "PaperZDAnimGraphNode_Transition.h"
#include "PaperZDEditor.h"
#include "PaperZDAnimGraphNode_State.h"

#include "PaperZDAnimTransitionGraph.h"
#include "PaperZDAnimTransitionGraphSchema.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EdGraphUtilities.h"
#include "AnimNodes/PaperZDAnimNode_Transition.h"

#include "Slate/SPaperZDAnimGraphNode_Transition.h"

//////////////////////////////////////////////////////////////////////////
//// PaperZD Anim Graph Node
//////////////////////////////////////////////////////////////////////////

UPaperZDAnimGraphNode_Transition::UPaperZDAnimGraphNode_Transition(const FObjectInitializer& ObjectInitializer)
	: Super(), Color(FColor::White)
{
	Priority = 0;
}

void UPaperZDAnimGraphNode_Transition::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetName().Equals(TEXT("Priority")))
	{
		Cast<UPaperZDAnimNode_Transition>(AnimNode)->Priority = Priority;
	}
}

void UPaperZDAnimGraphNode_Transition::CreateConnections(UPaperZDAnimGraphNode* PreviousState, UPaperZDAnimGraphNode* NextState)
{
	// Previous to this
	Pins[0]->Modify();
	Pins[0]->LinkedTo.Empty();
	
	PreviousState->GetOutputPin()->Modify();
	Pins[0]->MakeLinkTo(PreviousState->GetOutputPin());

	// This to next
	Pins[1]->Modify();
	Pins[1]->LinkedTo.Empty();

	NextState->GetInputPin()->Modify();
	Pins[1]->MakeLinkTo(NextState->GetInputPin());
}

void UPaperZDAnimGraphNode_Transition::AllocateDefaultPins()
{
	FCreatePinParams PinParams;
	UEdGraphPin* Inputs = CreatePin(EGPD_Input, TEXT("Transition"), TEXT(""), NULL, TEXT("In"), PinParams);
	Inputs->bHidden = true;
	UEdGraphPin* Outputs = CreatePin(EGPD_Output, TEXT("Transition"), TEXT(""), NULL, TEXT("Out"), PinParams);
	Outputs->bHidden = true;
}

void UPaperZDAnimGraphNode_Transition::PinConnectionListChanged(UEdGraphPin* Pin)
{
	if (Pin->LinkedTo.Num() == 0)
	{
		// Commit suicide; transitions must always have an input and output connection
		Modify();

		// Our parent graph will have our graph in SubGraphs so needs to be modified to record that.
		if (UEdGraph* ParentGraph = GetGraph())
		{
			ParentGraph->Modify();
		}

		DestroyNode();
	}
}
UPaperZDAnimGraphNode* UPaperZDAnimGraphNode_Transition::GetFromNode()
{
	if (!GetInputPin()->LinkedTo.Num())
		return nullptr;

	return Cast<UPaperZDAnimGraphNode>(GetInputPin()->LinkedTo[0]->GetOwningNode());
}

UPaperZDAnimGraphNode* UPaperZDAnimGraphNode_Transition::GetToNode()
{
	if (!GetOutputPin()->LinkedTo.Num())
		return nullptr;

	return Cast<UPaperZDAnimGraphNode>(GetOutputPin()->LinkedTo[0]->GetOwningNode());
}

TSharedPtr<SGraphNode> UPaperZDAnimGraphNode_Transition::CreateVisualWidget()
{
	return SNew(SPaperZDAnimGraphNode_Transition, this);
}
