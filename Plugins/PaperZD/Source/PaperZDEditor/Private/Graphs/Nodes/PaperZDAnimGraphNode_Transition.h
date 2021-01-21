// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once

#include "PaperZDAnimGraphNode_TransBase.h"
#include "PaperZDAnimGraphNode_Transition.generated.h"

class UPaperZDAnimSequence;
UCLASS()
class UPaperZDAnimGraphNode_Transition : public UPaperZDAnimGraphNode_TransBase
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Transition)
	int Priority;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Transition)
	FColor Color;

	UPROPERTY()
	FName SequenceIdentifier_DEPRECATED; //@DEPRECATED

	UPROPERTY(EditAnywhere, Category = _Animation)
	class UPaperZDAnimSequence* AnimSequence;

public:
	//~ Begin UEdGraphNode Interface
	virtual void AllocateDefaultPins() override;
	virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;
	virtual bool CanDuplicateNode() const override { return false; }
	virtual TSharedPtr<SGraphNode> CreateVisualWidget() override;
	//~ End UEdGraphNode Interface

	void CreateConnections(UPaperZDAnimGraphNode* PreviousState, UPaperZDAnimGraphNode* NextState);


	virtual class UPaperZDAnimGraphNode* GetFromNode();
	virtual class UPaperZDAnimGraphNode* GetToNode();

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
};
