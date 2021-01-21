// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once

#include "PaperZDanimGraphNode.h"
#include "PaperZDAnimGraphNode_Root.generated.h"

UCLASS()
class UPaperZDAnimGraphNode_Root : public UPaperZDAnimGraphNode
{
	GENERATED_UCLASS_BODY()

	//~ Begin UEdGraphNode Interface
	virtual void AllocateDefaultPins() override;
	virtual void AutowireNewNode(UEdGraphPin* FromPin) override;
	virtual bool CanDuplicateNode() const override { return false; }
	virtual bool CanUserDeleteNode() const override { return false; }
	virtual TSharedPtr<SGraphNode> CreateVisualWidget() override;
	//~ End UEdGraphNode Interface

	virtual UEdGraphPin* GetInputPin() override { return nullptr; }
	virtual UEdGraphPin* GetOutputPin() override { return Pins[0]; }
};
