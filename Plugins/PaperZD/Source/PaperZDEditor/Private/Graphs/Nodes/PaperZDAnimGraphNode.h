// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "EdGraph/EdGraphNode.h"
#include "PaperZDAnimGraphNode.generated.h"

UCLASS()
class UPaperZDAnimGraphNode : public UEdGraphNode
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY()
	class UPaperZDAnimNode *AnimNode;


public:
	virtual void SetAnimNode(class UPaperZDAnimNode *InAnimNode);
	class UPaperZDAnimNode* GetAnimNode() const { return AnimNode; };
	virtual void PostPasteNode() override;
	virtual void PostDuplicate(bool bDuplicateForPIE) override;

	virtual UEdGraphPin* GetInputPin() { return Pins[0]; }
	virtual UEdGraphPin* GetOutputPin() { return Pins[1]; }
};
