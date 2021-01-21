// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "PaperZDAnimGraphNode_Jump.h"
#include "PaperZDEditor.h"
#include "Graphs/PaperZDAnimGraph.h"
#include "PaperZDAnimBP.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/Kismet2NameValidators.h"

//Slate related
#include "Slate/SPaperZDAnimGraphNode_Jump.h"

//////////////////////////////////////////////////////////////////////////
//// Name Validator
//////////////////////////////////////////////////////////////////////////
class FPaperZDJumpNodeNameValidator : public FStringSetNameValidator
{
public:
	FPaperZDJumpNodeNameValidator(const UPaperZDAnimGraphNode_Jump* InJumpNode)
		: FStringSetNameValidator(FString())
	{
		TArray<UPaperZDAnimGraphNode_Jump*> Nodes;
		InJumpNode->GetGraph()->GetNodesOfClass(Nodes);

		for (UPaperZDAnimGraphNode_Jump* JNode : Nodes)
		{
			if (JNode != InJumpNode)
			{
				Names.Add(JNode->GetJumpName().ToString());
			}
		}
	}
};

//////////////////////////////////////////////////////////////////////////
//// Jump
//////////////////////////////////////////////////////////////////////////

UPaperZDAnimGraphNode_Jump::UPaperZDAnimGraphNode_Jump(const FObjectInitializer& ObjectInitializer)
	: Super()
{
	Name = TEXT("Jump");
	bCanRenameNode = true;
}

void UPaperZDAnimGraphNode_Jump::PostPlacedNewNode()
{
	Super::PostPlacedNewNode();

	//Must make sure the default name isn't used
	FPaperZDJumpNodeNameValidator Validator(this);
	FString TestName = Name.ToString();
	Validator.FindValidString(TestName);

	//Setup name
	Name = FName(*TestName);
}

void UPaperZDAnimGraphNode_Jump::AllocateDefaultPins()
{
	FCreatePinParams PinParams;
	CreatePin(EGPD_Output, TEXT("Transition"), TEXT(""), NULL, TEXT(""), PinParams);
}

TSharedPtr<SGraphNode> UPaperZDAnimGraphNode_Jump::CreateVisualWidget()
{ 
	return SNew(SPaperZDAnimGraphNode_Jump, this);
}

FText UPaperZDAnimGraphNode_Jump::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromName(Name);
}

void UPaperZDAnimGraphNode_Jump::OnRenameNode(const FString & NewName)
{
	Name = FName(*NewName);

	//Must mark the blueprint as need-compile, due to our name based indexing
	if (UPaperZDAnimGraph* Graph = Cast<UPaperZDAnimGraph>(GetGraph()))
	{
		FBlueprintEditorUtils::MarkBlueprintAsModified(Graph->GetAnimBP());
	}
}

FName UPaperZDAnimGraphNode_Jump::GetJumpName() const
{
	return Name;
}

TSharedPtr<INameValidatorInterface> UPaperZDAnimGraphNode_Jump::MakeNameValidator() const
{
	return MakeShareable(new FPaperZDJumpNodeNameValidator(this));
}
