// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "AnimNodes/PaperZDAnimNode_State.h"
#include "PaperZD.h"
#include "PaperZDCharacter.h"
#include "AnimNodes/PaperZDAnimNode_Transition.h"
#include "PaperZDAnimInstance.h"
#include "PaperFlipbook.h"
#include "PaperFlipbookComponent.h"
#include "AnimSequences/PaperZDAnimSequence.h"

#include "Notifies/PaperZDAnimNotify.h"
#include "Notifies/PaperZDAnimNotifyState.h"
#include "Notifies/PaperZDAnimNotifyCustom.h"
/////////////////////////////////////
//// State
/////////////////////////////////////
const int32 UPaperZDAnimNode_State::MaxTransitionalStateRecursion = 4;

UPaperZDAnimNode_State::UPaperZDAnimNode_State(const FObjectInitializer& ObjectInitializer)
	: Super()
{
	Type = EPaperZDNodeType::NODE_State;
	SetFlags(GetFlags() | RF_Transactional);
	bShouldLoop = true;
}

void UPaperZDAnimNode_State::Init(UPaperZDAnimInstance *OwningInstance)
{
	Super::Init(OwningInstance);

	Transitions.Sort([&](const UObject& LHS, const UObject& RHS) {
		const UPaperZDAnimNode_Transition *LeftTran = Cast<const UPaperZDAnimNode_Transition>(&LHS);
		const UPaperZDAnimNode_Transition *RightTran = Cast<const UPaperZDAnimNode_Transition>(&RHS);

		return LeftTran->Priority > RightTran->Priority;
	});
}

void UPaperZDAnimNode_State::Tick(float DeltaTime, UPaperZDAnimInstance *OwningInstance)
{
	TSet<const UPaperZDAnimNode*> VisitedNodes;
	ProcessTransitions(OwningInstance, VisitedNodes);
}

void UPaperZDAnimNode_State::ProcessTransitions(UPaperZDAnimInstance* OwningInstance, TSet<const UPaperZDAnimNode *> VisitedNodes, int32 NumRecursions)
{
	if (NumRecursions < MaxTransitionalStateRecursion)
	{
		VisitedNodes.Add(this);

		//Check for a valid transition while ticking them
		for (int32 i = 0; i < Transitions.Num(); i++)
		{
			UPaperZDAnimNode_Transition* t = Transitions[i];
			TSet<const UPaperZDAnimNode*> TransitionVisitedNodes = VisitedNodes;

			if (t->CanEnter(OwningInstance, TransitionVisitedNodes)) {

				//The transition should be entered before the target state
				t->Enter(OwningInstance);

				//Setup and re-tick for transitional states
				if (OwningInstance->AllowsTransitionalStates())
				{
					UPaperZDAnimNode_State* StateNode = Cast<UPaperZDAnimNode_State>(OwningInstance->CurrentState);
					if (StateNode)
					{
						StateNode->ProcessTransitions(OwningInstance, TransitionVisitedNodes, NumRecursions + 1);
					}
				}
				return;
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Max number of transitional state recursions reached on AnimInstance '%s'"), *OwningInstance->GetName());
	}
}

void UPaperZDAnimNode_State::Enter(UPaperZDAnimInstance* OwningInstance)
{
	OwningInstance->EnqueueAnimation(FAnimQueueInfo(AnimSequence, bShouldLoop, false));
	OwningInstance->CurrentState = this;
}

void UPaperZDAnimNode_State::ConnectOutputWith(TArray<UPaperZDAnimNode *>PossibleNodes)
{
	//Clear Transitions
	Transitions.Empty();

	for (auto Node : PossibleNodes)
	{
		Transitions.Add(CastChecked<UPaperZDAnimNode_Transition>(Node));
	}
}


TArray<UPaperZDAnimNode *> UPaperZDAnimNode_State::GetOutputNodes() const
{
	TArray<UPaperZDAnimNode *>Nodes;

	for (auto Node : Transitions)
	{
		Nodes.Add(Node);
	}

	return Nodes;
}
