// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "K2Node.h"
#include "PaperZDTransitionGraphNode_Result.generated.h"

UCLASS()
class UPaperZDTransitionGraphNode_Result : public UK2Node
{
	GENERATED_UCLASS_BODY()

public:

	//UEdGraphNode Interface
	virtual void AllocateDefaultPins() override;
	virtual bool CanUserDeleteNode() const override { return false; }
	virtual bool CanDuplicateNode() const override { return false; }
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	//End UEdGraphNodeInterface

	//UK2Node Interface
	virtual bool IsNodePure() const override { return true; }
};
