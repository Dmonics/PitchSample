// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "PaperZDAnimGraphNode_TransBase.h"
#include "PaperZDEditor.h"

#include "PaperZDAnimTransitionGraph.h"
#include "PaperZDAnimTransitionGraphSchema.h"
#include "EdGraphUtilities.h"

//Kismet2
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"

//////////////////////////////////////////////////////////////////////////
//// PaperZD Anim Graph Node
//////////////////////////////////////////////////////////////////////////

UPaperZDAnimGraphNode_TransBase::UPaperZDAnimGraphNode_TransBase(const FObjectInitializer& ObjectInitializer)
	: Super()
{
}

void UPaperZDAnimGraphNode_TransBase::SetAnimNode(class UPaperZDAnimNode *InAnimNode)
{
	Super::SetAnimNode(InAnimNode);

	//Must deep replace the AnimNode
	if (UPaperZDAnimTransitionGraph *Graph = Cast<UPaperZDAnimTransitionGraph>(BoundGraph))
		Graph->TransitionNode = InAnimNode;
}

void UPaperZDAnimGraphNode_TransBase::CreateBoundGraph()
{
	// Create a new animation graph
	check(BoundGraph == NULL);
	BoundGraph = FBlueprintEditorUtils::CreateNewGraph(this, NAME_None, UPaperZDAnimTransitionGraph::StaticClass(), UPaperZDAnimTransitionGraphSchema::StaticClass());
	check(BoundGraph);

	BoundGraph->bAllowDeletion = false;

	//Add the transition node for further info
	if (UPaperZDAnimTransitionGraph *Graph = CastChecked<UPaperZDAnimTransitionGraph>(BoundGraph))
		Graph->TransitionNode = AnimNode;

	// Find an interesting name
	FEdGraphUtilities::RenameGraphToNameOrCloseToName(BoundGraph, TEXT("Transition"));

	// Initialize the anim graph
	const UEdGraphSchema* Schema = BoundGraph->GetSchema();
	Schema->CreateDefaultNodesForGraph(*BoundGraph);

	// Add the new graph as a child of our parent graph
	UEdGraph* ParentGraph = GetGraph();

	if (ParentGraph->SubGraphs.Find(BoundGraph) == INDEX_NONE)
	{
		ParentGraph->SubGraphs.Add(BoundGraph);
	}
}

void UPaperZDAnimGraphNode_TransBase::PostPlacedNewNode()
{
	CreateBoundGraph();
}

void UPaperZDAnimGraphNode_TransBase::DestroyNode()
{
	// BoundGraph may be shared with another graph, if so, don't remove it here
	UEdGraph* GraphToRemove = GetBoundGraph();
	ClearBoundGraph();
	
	Super::DestroyNode();

	if (GraphToRemove)
	{
		UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForNodeChecked(this);
		FBlueprintEditorUtils::RemoveGraph(Blueprint, GraphToRemove, EGraphRemoveFlags::Recompile);
	}
}

UObject* UPaperZDAnimGraphNode_TransBase::GetJumpTargetForDoubleClick() const
{
	return GetBoundGraph();
}

void UPaperZDAnimGraphNode_TransBase::JumpToDefinition() const
{
	FKismetEditorUtilities::BringKismetToFocusAttentionOnObject(GetJumpTargetForDoubleClick());
}

void UPaperZDAnimGraphNode_TransBase::ClearBoundGraph()
{
	// Add the new graph as a child of our parent graph
	UEdGraph* ParentGraph = GetGraph();

	int32 index = ParentGraph->SubGraphs.Find(BoundGraph);
	if (index != INDEX_NONE)
	{
		ParentGraph->SubGraphs.RemoveAt(index);
	}

	BoundGraph = nullptr;
}
