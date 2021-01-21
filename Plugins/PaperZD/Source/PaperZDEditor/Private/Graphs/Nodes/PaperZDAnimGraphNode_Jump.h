// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once

#include "PaperZDanimGraphNode.h"
#include "PaperZDAnimGraphNode_Jump.generated.h"

/**
 * Specialized node that works as a secondary root and can be called from the AnimInstance to force change the flow of the state machine.
 * Will keep connected nodes to it for being cleaned up on the compile process, like the root node.
 */
UCLASS()
class UPaperZDAnimGraphNode_Jump : public UPaperZDAnimGraphNode
{
	GENERATED_BODY()

	/* Name for this redirector, will be indexed on the AnimInstance. */
	UPROPERTY()
	FName Name;

public:
	UPaperZDAnimGraphNode_Jump(const FObjectInitializer& ObjectInitializer);

	//~ Begin UEdGraphNode Interface
	virtual void AllocateDefaultPins() override;
	virtual TSharedPtr<SGraphNode> CreateVisualWidget() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual void OnRenameNode(const FString & NewName) override;
	virtual TSharedPtr <class INameValidatorInterface> MakeNameValidator() const override;
	virtual void PostPlacedNewNode() override;
	virtual bool CanDuplicateNode() const override { return false; }
	//~ End UEdGraphNode Interface
	
	virtual UEdGraphPin* GetInputPin() override { return nullptr; }
	virtual UEdGraphPin* GetOutputPin() override { return Pins[0]; }

	/* Obtains the name of this jump node. */
	FName GetJumpName() const;
};
