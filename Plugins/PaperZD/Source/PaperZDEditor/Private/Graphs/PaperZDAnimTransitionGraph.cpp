// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "PaperZDAnimTransitionGraph.h"
#include "PaperZDEditor.h"
#include "Graphs/Nodes/PaperZDTransitionGraphNode_Result.h"

//////////////////////////////////////////////////////////////////////////
//// PaperZD Anim Graph
//////////////////////////////////////////////////////////////////////////
UPaperZDAnimTransitionGraph::UPaperZDAnimTransitionGraph(const FObjectInitializer& ObjectInitializer)
	: Super()
{
}

UPaperZDTransitionGraphNode_Result* UPaperZDAnimTransitionGraph::GetResultNode()
{
	//@TODO: Will change this into a proper getter/setter
	return CastChecked<UPaperZDTransitionGraphNode_Result>(Nodes[0]);
}