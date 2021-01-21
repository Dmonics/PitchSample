// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "EdGraph/EdGraph.h"
#include "AnimNodes/PaperZDAnimNode.h"
#include "PaperZDAnimTransitionGraph.generated.h"


UCLASS()
class UPaperZDAnimTransitionGraph : public UEdGraph
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY()
	UPaperZDAnimNode *TransitionNode;

	class UPaperZDTransitionGraphNode_Result* GetResultNode();
};
