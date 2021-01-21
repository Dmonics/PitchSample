// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "EdGraphSchema_K2.h"
#include "PaperZDAnimTransitionGraphSchema.generated.h"

UCLASS()
class UPaperZDAnimTransitionGraphSchema : public UEdGraphSchema_K2
{
	GENERATED_UCLASS_BODY()
	
	// UEdSchema Interface //
	virtual void CreateDefaultNodesForGraph(UEdGraph& Graph) const override;
	virtual bool CreateAutomaticConversionNodeAndConnections(UEdGraphPin* PinA, UEdGraphPin* PinB) const override;
	// UEdSchema Interface //

	virtual EGraphType GetGraphType(const UEdGraph* TestEdGraph) const { return GT_StateMachine; }
};
