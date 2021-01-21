// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once

#include "AnimNodes/PaperZDAnimNode.h"
#include "PaperZDAnimNode_Root.generated.h"

UCLASS()
class PAPERZD_API UPaperZDAnimNode_Root : public UPaperZDAnimNode
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY()
	class UPaperZDAnimNode_State* TargetState;

public:
	virtual void Tick(float DeltaTime, UPaperZDAnimInstance *OwningInstance) override;
	virtual void ConnectOutputWith(TArray<UPaperZDAnimNode *>PossibleNodes) override;
	virtual TArray<UPaperZDAnimNode *> GetOutputNodes() const override;
};
