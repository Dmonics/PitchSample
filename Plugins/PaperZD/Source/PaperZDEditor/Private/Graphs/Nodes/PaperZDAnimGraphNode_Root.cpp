// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "PaperZDAnimGraphNode_Root.h"
#include "PaperZDEditor.h"

//Slate related
#include "Slate/SPaperZDAnimGraphNode_Root.h"

//////////////////////////////////////////////////////////////////////////
//// PaperZD Anim Graph Node
//////////////////////////////////////////////////////////////////////////

UPaperZDAnimGraphNode_Root::UPaperZDAnimGraphNode_Root(const FObjectInitializer& ObjectInitializer)
	: Super()
{
}

void UPaperZDAnimGraphNode_Root::AllocateDefaultPins()
{
	FCreatePinParams PinParams;
	CreatePin(EGPD_Output, TEXT("Transition"), TEXT(""), NULL, TEXT("Out"), PinParams);
}

void UPaperZDAnimGraphNode_Root::AutowireNewNode(UEdGraphPin* FromPin)
{

}

TSharedPtr<SGraphNode> UPaperZDAnimGraphNode_Root::CreateVisualWidget() 
{ 
	return SNew(SPaperZDAnimGraphNode_Root, this);
}