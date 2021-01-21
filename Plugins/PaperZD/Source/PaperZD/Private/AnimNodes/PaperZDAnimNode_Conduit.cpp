// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "AnimNodes/PaperZDAnimNode_Conduit.h"
#include "PaperZD.h"
#include "AnimNodes/PaperZDAnimNode_Transition.h"
//#include "CoreUObject.h"


UPaperZDAnimNode_Conduit::UPaperZDAnimNode_Conduit(const FObjectInitializer& ObjectInitializer)
	: Super()
{
	Type = EPaperZDNodeType::NODE_Conduit;
}

void UPaperZDAnimNode_Conduit::Init(UPaperZDAnimInstance *OwningInstance)
{
	Super::Init(OwningInstance);

	Transitions.Sort([&](const UObject& LHS, const UObject& RHS) {
		const UPaperZDAnimNode_Transition *LeftTran = Cast<const UPaperZDAnimNode_Transition>(&LHS);
		const UPaperZDAnimNode_Transition *RightTran = Cast<const UPaperZDAnimNode_Transition>(&RHS);

		return LeftTran->Priority > RightTran->Priority;
	});
}

void UPaperZDAnimNode_Conduit::ConnectOutputWith(TArray<UPaperZDAnimNode *>PossibleNodes)
{
	//Clear Transitions
	Transitions.Empty();

	for (auto Node : PossibleNodes)
	{
		Transitions.Add(CastChecked<UPaperZDAnimNode_Transition>(Node));
	}
}


TArray<UPaperZDAnimNode *> UPaperZDAnimNode_Conduit::GetOutputNodes() const
{
	TArray<UPaperZDAnimNode *>Nodes;

	for (auto Node : Transitions)
	{
		Nodes.Add(Node);
	}

	return Nodes;
}
