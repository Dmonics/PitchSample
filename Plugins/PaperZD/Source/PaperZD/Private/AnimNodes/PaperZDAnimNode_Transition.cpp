// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "AnimNodes/PaperZDAnimNode_Transition.h"
#include "PaperZD.h"
//#include "CoreUObject.h"


UPaperZDAnimNode_Transition::UPaperZDAnimNode_Transition(const FObjectInitializer& ObjectInitializer)
	: Super()
{
	Type = EPaperZDNodeType::NODE_Transition;
}

void UPaperZDAnimNode_Transition::ConnectOutputWith(TArray<UPaperZDAnimNode *>PossibleNodes)
{
	//No more than 1 root connection (can be 0 if no connection has been installed)
	check(PossibleNodes.Num() < 2);

	if (PossibleNodes.Num())
		TargetNode = PossibleNodes[0];
}


TArray<UPaperZDAnimNode *> UPaperZDAnimNode_Transition::GetOutputNodes() const
{
	TArray<UPaperZDAnimNode *>Nodes;
	Nodes.Add(TargetNode);
	return Nodes;
}

void UPaperZDAnimNode_Transition::Enter(UPaperZDAnimInstance *OwningInstance)
{
	//Before transitioning to our target, we should first add the AnimTransition
	OwningInstance->ClearAnimationQueue();
	OwningInstance->EnqueueAnimation(FAnimQueueInfo(AnimSequence));

	//Now we can continue
	Super::Enter(OwningInstance);
}
