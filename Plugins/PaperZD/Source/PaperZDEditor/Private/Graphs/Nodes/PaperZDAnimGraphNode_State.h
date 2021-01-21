// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once

#include "PaperZDanimGraphNode.h"
#include "PaperZDAnimGraphNode_State.generated.h"

class UPaperZDAnimSequence;

/**
 * The state node is the main type of nodes for the state machine. In general, every other type of node is just transitory, while the StateNode is the only one
 * that can be active for more than one frame.
 */
UCLASS()
class UPaperZDAnimGraphNode_State : public UPaperZDAnimGraphNode
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY()
	FText Name;

	UPROPERTY()
	class UPaperFlipbook* Flipbook_DEPRECATED; //@Deprecated

	UPROPERTY()
	FName SequenceIdentifier_DEPRECATED; //@Deprecated

	UPROPERTY(EditAnywhere, Category = _Animation)
	UPaperZDAnimSequence* AnimSequence;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Playback)
	bool bShouldLoop;


	//~ Begin UObject Interface
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	//~End UObject Interface

	//~ Begin UEdGraphNode Interface
	virtual void AllocateDefaultPins() override;
	virtual void AutowireNewNode(UEdGraphPin* FromPin) override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual void OnRenameNode(const FString & NewName) override;
	virtual UObject* GetJumpTargetForDoubleClick() const override;
	virtual TSharedPtr<SGraphNode> CreateVisualWidget() override;
	virtual TSharedPtr <class INameValidatorInterface> MakeNameValidator() const override;
	virtual bool CanJumpToDefinition() const override { return true; }
	virtual void JumpToDefinition() const override;
	//~ End UEdGraphNode Interface

	TArray<class UPaperZDAnimGraphNode_Transition *>GetTransitions();

	FString GetStateName() { return Name.ToString(); }
};
