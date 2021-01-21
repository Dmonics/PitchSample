// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once

#include "AnimNodes/PaperZDAnimNode.h"
#include "AnimNodes/PaperZDAnimNode_TransBase.h"
#include "PaperZDAnimNode_Transition.generated.h"

class UPaperZDAnimSequence;
UCLASS()
class PAPERZD_API UPaperZDAnimNode_Transition : public UPaperZDAnimNode_TransBase
{
	GENERATED_UCLASS_BODY()

public:
	//Can be a AnimNodeState or AnimNodeConduit as of now
	UPROPERTY()
	class UPaperZDAnimNode *TargetNode;

	UPROPERTY()
	int Priority;

	//The AnimSequence isn't stored for some reason, so we store the identifier and recover the sequence on Init
	/*UPROPERTY()
	FName SequenceIdentifier;*/

	UPROPERTY(/*Transient*/)
	UPaperZDAnimSequence* AnimSequence;

public:
	virtual void ConnectOutputWith(TArray<UPaperZDAnimNode *>PossibleNodes) override;
	virtual TArray<UPaperZDAnimNode *> GetOutputNodes() const; 
	virtual void Enter(UPaperZDAnimInstance *OwningInstance) override;
};
