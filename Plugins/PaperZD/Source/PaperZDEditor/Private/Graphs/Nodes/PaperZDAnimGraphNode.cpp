// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "PaperZDAnimGraphNode.h"
#include "PaperZDEditor.h"
#include "AnimNodes/PaperZDAnimNode.h"
#include "PaperZDAnimBP.h"

//////////////////////////////////////////////////////////////////////////
//// PaperZD Anim Graph Node
//////////////////////////////////////////////////////////////////////////

UPaperZDAnimGraphNode::UPaperZDAnimGraphNode(const FObjectInitializer& ObjectInitializer)
	: Super()
{
}

void UPaperZDAnimGraphNode::SetAnimNode(UPaperZDAnimNode *InAnimNode)
{
	AnimNode = InAnimNode;
	InAnimNode->GraphNode = this;
}

void UPaperZDAnimGraphNode::PostPasteNode()
{
	Super::PostPasteNode();
	
	//Make sure the AnimNode is deep copied
	AnimNode = DuplicateObject(AnimNode, this); //Changed from AnimNode->GetOuter(), because that won't work when copying an external node
	AnimNode->GraphNode = this;
}

void UPaperZDAnimGraphNode::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);

	//Duplicate the AnimNode and update it's GraphNode
	AnimNode = DuplicateObject(AnimNode, this); 
	AnimNode->GraphNode = this;
}
