// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "AnimNodes/PaperZDAnimNode_Jump.h"
#include "PaperZD.h"
#include "AnimNodes/PaperZDAnimNode_State.h"


UPaperZDAnimNode_Jump::UPaperZDAnimNode_Jump(const FObjectInitializer& ObjectInitializer)
	: Super()
{
	Type = EPaperZDNodeType::NODE_Jump;
}

void UPaperZDAnimNode_Jump::Enter(UPaperZDAnimInstance* OwningInstance)
{
	Super::Enter(OwningInstance);

	if (ensureMsgf(TargetState, TEXT("No State node set after Redirector Node, AnimBP will not work after jump")))
	{
		//Must clear AnimationQueue before triggering (otherwise, queue will never go to next animation, due to it being a queued state which loops)
		OwningInstance->ClearAnimationQueue();

		//Finally ask the target state to enter
		TargetState->Enter(OwningInstance);
	}
}

void UPaperZDAnimNode_Jump::ConnectOutputWith(TArray<UPaperZDAnimNode *>PossibleNodes)
{
	//No more than 1 root connection (can be 0 if no connection has been installed)
	check(PossibleNodes.Num() < 2);
	if (PossibleNodes.Num())
	{
		TargetState = Cast<UPaperZDAnimNode_State>(PossibleNodes[0]);
	}
	else
	{
		TargetState = nullptr; //Don't forget to erase the target state if it was deleted (to avoid saving via UPROPERTY undesired states)
	}
}


TArray<UPaperZDAnimNode *> UPaperZDAnimNode_Jump::GetOutputNodes() const
{	
	//Check that the target state is set, if not we would be passing a nullptr
	TArray<UPaperZDAnimNode*>Nodes;
	if (TargetState)
	{
		Nodes.Add(TargetState);
	}

	return Nodes;
}
