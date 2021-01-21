// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "PaperZDAnimTransitionGraphSchema.h"
#include "PaperZDEditor.h"
#include "PaperZDAnimTransitionGraph.h"
#include "PaperZDTransitionGraphNode_Result.h"

//Editor
#include "ScopedTransaction.h"

//Nodes
#include "Kismet2/BlueprintEditorUtils.h"

#define LOCTEXT_NAMESPACE "PaperZDAnimGraphSchema"

//////////////////////////////////////////////////////////////////////////
//// PaperZD Anim Graph Schema
//////////////////////////////////////////////////////////////////////////
UPaperZDAnimTransitionGraphSchema::UPaperZDAnimTransitionGraphSchema(const FObjectInitializer& ObjectInitializer)
	: Super()
{
}

void UPaperZDAnimTransitionGraphSchema::CreateDefaultNodesForGraph(UEdGraph& Graph) const
{
	FGraphNodeCreator<UPaperZDTransitionGraphNode_Result> NodeCreator(Graph);
	UPaperZDTransitionGraphNode_Result* ResultSinkNode = NodeCreator.CreateNode();
	NodeCreator.Finalize();
	SetNodeMetaData(ResultSinkNode, FNodeMetadata::DefaultGraphNode);
}

bool UPaperZDAnimTransitionGraphSchema::CreateAutomaticConversionNodeAndConnections(UEdGraphPin* PinA, UEdGraphPin* PinB) const
{
	return false;
}
#undef LOCTEXT_NAMESPACE