// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateColor.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SNodePanel.h"
#include "SGraphNode.h"

class SGraphPin;
class UPaperZDAnimGraphNode_Jump;

class SPaperZDAnimGraphNode_Jump : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SPaperZDAnimGraphNode_Jump) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UPaperZDAnimGraphNode_Jump* InNode);
	
	// SGraphNode interface
	virtual void UpdateGraphNode() override;
	virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
	// End of SGraphNode interface


protected:
	FSlateColor GetBorderBackgroundColor() const;

	FText GetPreviewCornerText() const;
};
