// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once

#include "AnimNodes/PaperZDAnimNode.h"
#include "PaperZDAnimNode_TransBase.generated.h"

UCLASS()
class PAPERZD_API UPaperZDAnimNode_TransBase : public UPaperZDAnimNode
{
	GENERATED_UCLASS_BODY()

protected:
	/* Variable only meant to be shown as a pin. */
	UPROPERTY(Transient, meta=(ShowAsNodePin))
	bool bCanEnterTransition;

	UPROPERTY()
	mutable TWeakObjectPtr<UPaperZDAnimNode> CachedTarget;

public:
	UPROPERTY()
	FName FunctionName;

private:
	UPROPERTY(Transient)
	UFunction *BoundFunction;

public:
	virtual void Init(UPaperZDAnimInstance *OwningInstance) override;
	virtual void Enter(UPaperZDAnimInstance *OwningInstance) override;
	virtual bool CanEnter(UPaperZDAnimInstance *OwningInstance, TSet<const UPaperZDAnimNode*>& VisitedNodes) const override;
};
