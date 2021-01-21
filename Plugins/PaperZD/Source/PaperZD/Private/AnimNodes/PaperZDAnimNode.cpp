// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "AnimNodes/PaperZDAnimNode.h"
#include "PaperZD.h"
#include "Notifies/PaperZDAnimNotify_Base.h"

UPaperZDAnimNode::UPaperZDAnimNode(const FObjectInitializer& ObjectInitializer)
	: Super()
{
}

#if WITH_EDITOR
TArray<UProperty *> UPaperZDAnimNode::GetPinProperties()
{
	TArray<UProperty *> PropertyArray;
	for (TFieldIterator<UProperty> PropIt(GetClass(), EFieldIteratorFlags::IncludeSuper); PropIt; ++PropIt)
	{
		if(PropIt->HasMetaData(FName("ShowAsNodePin")))
			PropertyArray.Add(*PropIt);
	}

	return PropertyArray;
}
#endif

void UPaperZDAnimNode::PropagateInit(UPaperZDAnimInstance *OwningInstance, TSet<UPaperZDAnimNode *>& VisitedNodes)
{
	//Check if the node already was visited
	if (VisitedNodes.Contains(this))
	{
		return;
	}

	//Must add to the visited node list and ask to initialize
	VisitedNodes.Add(this);
	Init(OwningInstance);

	for (auto Node : GetOutputNodes())
	{
		Node->PropagateInit(OwningInstance, VisitedNodes);
	}
}

bool UPaperZDAnimNode::CanEnter(UPaperZDAnimInstance* OwningInstance, TSet<const UPaperZDAnimNode*>& VisitedNodes) const
{
	if (VisitedNodes.Contains(this))
	{
		UE_LOG(LogTemp, Warning, TEXT("Reached closed loop recursion when checking for AnimNode transitions. States could fickle due to having non deterministic transition rules. OwningInstance: '%s'"), *OwningInstance->GetName())
		return false;
	}
	else
	{
		VisitedNodes.Add(this);
		return true;
	}
}
