// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "AnimNodes/PaperZDAnimNode_Root.h"
#include "PaperZD.h"
#include "AnimNodes/PaperZDAnimNode_State.h"


UPaperZDAnimNode_Root::UPaperZDAnimNode_Root(const FObjectInitializer& ObjectInitializer)
	: Super()
{
	Type = EPaperZDNodeType::NODE_Root;
}

void UPaperZDAnimNode_Root::Tick(float DeltaTime, UPaperZDAnimInstance *OwningInstance)
{
	//Must have a TargetState
	if (ensureMsgf(TargetState, TEXT("No State node set after Root Node, AnimBP will not work with no states set")))
	{
		OwningInstance->CurrentState = TargetState;
		TargetState->Enter(OwningInstance);

		//This node is just used as a bridge, should not use a tick period on its entirety
		TargetState->Tick(DeltaTime, OwningInstance);
	}
}

void UPaperZDAnimNode_Root::ConnectOutputWith(TArray<UPaperZDAnimNode *>PossibleNodes)
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


TArray<UPaperZDAnimNode *> UPaperZDAnimNode_Root::GetOutputNodes() const
{	
	//Check that the target state is set, if not we would be passing a nullptr
	TArray<UPaperZDAnimNode*>Nodes;
	if (TargetState)
	{
		Nodes.Add(TargetState);
	}

	return Nodes;
}
