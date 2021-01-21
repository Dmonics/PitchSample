// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "PaperZDTransitionGraphNode_Result.h"
#include "PaperZDEditor.h"
#include "GraphEditorSettings.h"
#include "AnimNodes/PaperZDAnimNode_Transition.h"
#include "PaperZDAnimTransitionGraph.h"
#include "EdGraphSchema_K2.h"

//////////////////////////////////////////////////////////////////////////
//// PaperZD Anim Graph Node
//////////////////////////////////////////////////////////////////////////
#define LOCTEXT_NAMESPACE "PaperZDTransitionGraphNode_Result"
UPaperZDTransitionGraphNode_Result::UPaperZDTransitionGraphNode_Result(const FObjectInitializer& ObjectInitializer)
	: Super()
{
}

void UPaperZDTransitionGraphNode_Result::AllocateDefaultPins()
{
	UPaperZDAnimTransitionGraph *Graph = CastChecked<UPaperZDAnimTransitionGraph>(GetGraph());
	
	TArray<UProperty *> Properties = Graph->TransitionNode->GetPinProperties();

	//Iterate and create the pins by using the GetPinProperties method of the schema
	const UEdGraphSchema_K2 *K2Schema = Cast<const UEdGraphSchema_K2>(GetGraph()->GetSchema());

	for(int i = 0; i < Properties.Num(); i++)
	{
		FEdGraphPinType Type;
		UProperty *prop = Properties[i];
		K2Schema->ConvertPropertyToPinType(prop, Type);

		CreatePin(EGPD_Input, Type, prop->GetFName());
	}
}

FLinearColor UPaperZDTransitionGraphNode_Result::GetNodeTitleColor() const
{
	return GetDefault<UGraphEditorSettings>()->ResultNodeTitleColor;
}

FText UPaperZDTransitionGraphNode_Result::GetTooltipText() const
{
	return LOCTEXT("TransitionResultTooltip", "This expression is evaluated to determine if the state transition can be taken");
}

FText UPaperZDTransitionGraphNode_Result::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("Result", "Result");
}

#undef LOCTEXT_NAMESPACE
