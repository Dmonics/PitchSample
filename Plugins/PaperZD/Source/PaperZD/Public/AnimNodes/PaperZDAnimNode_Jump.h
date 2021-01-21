// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once

#include "AnimNodes/PaperZDAnimNode.h"
#include "PaperZDAnimNode_Jump.generated.h"

class UPaperZDAnimNode_State;

/**
 * Specialized runtime node that works as an extra root node that can be redirected to.
 */
UCLASS()
class PAPERZD_API UPaperZDAnimNode_Jump : public UPaperZDAnimNode
{
	GENERATED_BODY()

public:
	/* Target state to go to. */
	UPROPERTY()
	UPaperZDAnimNode_State* TargetState;

public:
	UPaperZDAnimNode_Jump(const FObjectInitializer& ObjectInitializer);

	virtual void ConnectOutputWith(TArray<UPaperZDAnimNode *>PossibleNodes) override;
	virtual TArray<UPaperZDAnimNode *> GetOutputNodes() const override;
	virtual void Enter(UPaperZDAnimInstance* OwningInstance) override;
};
