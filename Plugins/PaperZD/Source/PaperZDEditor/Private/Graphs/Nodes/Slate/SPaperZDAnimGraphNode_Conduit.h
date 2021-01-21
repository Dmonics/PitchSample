// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateColor.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SNodePanel.h"
#include "SGraphNode.h"
#include "SGraphPin.h"

//
// Forward declarations.
//
class UPaperZDAnimGraphNode_Conduit;
class SToolTip;

class SPaperZDAnimGraphNode_Conduit : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SPaperZDAnimGraphNode_Conduit) {}
	SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, UPaperZDAnimGraphNode_Conduit* InNode);

	
	// SNodePanel::SNode interface
	virtual void GetNodeInfoPopups(FNodeInfoContext* Context, TArray<FGraphInformationPopupInfo>& Popups) const override;
	// End of SNodePanel::SNode interface

	// SGraphNode interface
	virtual void UpdateGraphNode() override;
	virtual void CreatePinWidgets() override;
	virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
	// End of SGraphNode interface

	static void GetStateInfoPopup(UEdGraphNode* GraphNode, TArray<FGraphInformationPopupInfo>& Popups);
protected:
	FSlateColor GetBorderBackgroundColor() const;

	//virtual FText GetPreviewCornerText() const;
	virtual const FSlateBrush* GetNameIcon() const;
};