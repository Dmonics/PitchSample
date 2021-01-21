// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateColor.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SWidget.h"
#include "SNodePanel.h"
#include "SGraphNode.h"

class SToolTip;
class UPaperZDAnimGraphNode_Transition;

class SPaperZDAnimGraphNode_Transition : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SPaperZDAnimGraphNode_Transition) {}
	SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, UPaperZDAnimGraphNode_Transition* InNode);

	// SNodePanel::SNode interface
	virtual void GetNodeInfoPopups(FNodeInfoContext* Context, TArray<FGraphInformationPopupInfo>& Popups) const override;
	virtual void MoveTo(const FVector2D& NewPosition, FNodeSet& NodeFilter) override;
	virtual bool RequiresSecondPassLayout() const override;
	virtual void PerformSecondPassLayout(const TMap< UObject*, TSharedRef<SNode> >& NodeToWidgetLookup) const override;
	// End of SNodePanel::SNode interface

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime);
	const FSlateBrush *GetBodyBrush() const;

	// SGraphNode interface
	virtual void UpdateGraphNode() override;
	virtual TSharedPtr<SToolTip> GetComplexTooltip() override;
	// End of SGraphNode interface

	// Calculate position for multiple nodes to be placed between a start and end point, by providing this nodes index and max expected nodes 
	void PositionBetweenTwoNodesWithOffset(const FGeometry& StartGeom, const FGeometry& EndGeom, int32 NodeIndex, int32 MaxNodes) const;

	static FLinearColor StaticGetTransitionColor(UPaperZDAnimGraphNode_Transition* TransNode, bool bIsHovered);
private:
	TSharedPtr<STextEntryPopup> TextEntryWidget;

private:
	FText GetPreviewCornerText(bool reverse) const;
	FSlateColor GetTransitionColor() const;
	class UPaperZDAnimGraphNode_Transition *TransitionNode;
	bool bNodeHadValidSequenceConfigured;

	TSharedRef<SWidget> GenerateRichTooltip();
};
