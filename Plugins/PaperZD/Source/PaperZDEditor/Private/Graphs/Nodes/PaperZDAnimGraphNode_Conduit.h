// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once

#include "PaperZDanimGraphNode_TransBase.h"
#include "PaperZDAnimGraphNode_Conduit.generated.h"

UCLASS()
class UPaperZDAnimGraphNode_Conduit : public UPaperZDAnimGraphNode_TransBase
{
	GENERATED_UCLASS_BODY()

private:
	UPROPERTY()
	FText Name;

public:
	//~ Begin UEdGraphNode Interface
	virtual void AllocateDefaultPins() override;
	virtual void AutowireNewNode(UEdGraphPin* FromPin) override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual void OnRenameNode(const FString & NewName) override;
	virtual TSharedPtr<SGraphNode> CreateVisualWidget() override;
	virtual TSharedPtr <class INameValidatorInterface> MakeNameValidator() const override;
	//~ End UEdGraphNode Interface


	FString GetNameAsString() { return Name.ToString(); }
	TArray<class UPaperZDAnimGraphNode_Transition *>GetTransitions();
};
