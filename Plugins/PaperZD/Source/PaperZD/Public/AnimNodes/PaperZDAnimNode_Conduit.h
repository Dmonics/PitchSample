// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once

#include "AnimNodes/PaperZDAnimNode_TransBase.h"
#include "PaperZDAnimNode_Conduit.generated.h"

UCLASS()
class PAPERZD_API UPaperZDAnimNode_Conduit : public UPaperZDAnimNode_TransBase
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY()
	TArray<class UPaperZDAnimNode_Transition*>Transitions;

	//~UPaperZDAnimNode
	virtual void Init(UPaperZDAnimInstance *OwningInstance) override;
	virtual void ConnectOutputWith(TArray<UPaperZDAnimNode *>PossibleNodes) override;
	virtual TArray<UPaperZDAnimNode *> GetOutputNodes() const;
	//~UPaperZDAnimNode 

};
