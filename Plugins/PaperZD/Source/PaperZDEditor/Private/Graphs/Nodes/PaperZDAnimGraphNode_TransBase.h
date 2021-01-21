// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once

#include "PaperZDanimGraphNode.h"
#include "PaperZDAnimGraphNode_TransBase.generated.h"

class UPaperZDAnimSequence;
UCLASS()
class UPaperZDAnimGraphNode_TransBase : public UPaperZDAnimGraphNode
{
	GENERATED_UCLASS_BODY()
		
private:
	UPROPERTY()
	UEdGraph *BoundGraph;

public:
	//~ Begin UEdGraphNode Interface
	virtual void PostPlacedNewNode() override;
	virtual void DestroyNode() override;
	virtual UObject* GetJumpTargetForDoubleClick() const override;
	virtual bool CanJumpToDefinition() const override { return true; }
	virtual void JumpToDefinition() const override;
	//~ End UEdGraphNode Interface


	//Setup the AnimNode
	virtual void SetAnimNode(class UPaperZDAnimNode *InAnimNode) override;

	//TransBase
	void CreateBoundGraph();
	UEdGraph* GetBoundGraph() const { return BoundGraph; }
	void ClearBoundGraph();
	//TransBase
};
