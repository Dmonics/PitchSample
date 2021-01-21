// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "EdGraph/EdGraphSchema.h"
#include "PaperZDAnimGraphSchema.generated.h"

/** Action to add a node to the graph */
class UPaperZDAnimNode;
USTRUCT()
struct FPaperZDAnimGraphSchemaAction_NewNode : public FEdGraphSchemaAction
{
	GENERATED_BODY();

	/** Class of node we want to create */
	UPROPERTY()
		UClass* AnimNodeClass;


	FPaperZDAnimGraphSchemaAction_NewNode()
		: FEdGraphSchemaAction()
		, AnimNodeClass(NULL)
	{}

	FPaperZDAnimGraphSchemaAction_NewNode(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping)
		: FEdGraphSchemaAction(MoveTemp(InNodeCategory), MoveTemp(InMenuDesc), MoveTemp(InToolTip), InGrouping)
		, AnimNodeClass(NULL)
	{}

	//~ Begin FEdGraphSchemaAction Interface
	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	//~ End FEdGraphSchemaAction Interface
	
	template<typename T>
	static T* SpawnGraphNodeFromAnimNode(class UEdGraph* ParentGraph, /*TSubclassOf<UPaperZDAnimNode>*/ UClass *InAnimNodeClass, const FVector2D Location = FVector2D(0.0f, 0.0f), bool bSelectNewNode = true)
	{
		FPaperZDAnimGraphSchemaAction_NewNode Action;
		Action.AnimNodeClass = InAnimNodeClass;

		return Cast<T>(Action.PerformAction(ParentGraph, NULL, Location, bSelectNewNode));
	}
};

USTRUCT()
struct FPaperZDAnimGraphSchemaAction_NewComment : public FEdGraphSchemaAction
{
	GENERATED_BODY();

	FPaperZDAnimGraphSchemaAction_NewComment()
		: FEdGraphSchemaAction()
	{}

	FPaperZDAnimGraphSchemaAction_NewComment(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping)
		: FEdGraphSchemaAction(MoveTemp(InNodeCategory), MoveTemp(InMenuDesc), MoveTemp(InToolTip), InGrouping)
	{}

	//~ Begin FEdGraphSchemaAction Interface
	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	//~ End FEdGraphSchemaAction Interface
};


UCLASS()
class UPaperZDAnimGraphSchema : public UEdGraphSchema
{
	GENERATED_BODY()

public:
	UPaperZDAnimGraphSchema(const FObjectInitializer& ObjectInitializer);

	// UEdSchema Interface //
	virtual void CreateDefaultNodesForGraph(UEdGraph& Graph) const override;
	virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override; 
	virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const override;
	virtual bool TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const override;
	virtual bool CreateAutomaticConversionNodeAndConnections(UEdGraphPin* PinA, UEdGraphPin* PinB) const override;
	virtual void GetGraphDisplayInformation(const UEdGraph & Graph, FGraphDisplayInfo & DisplayInfo) const override;
	virtual class FConnectionDrawingPolicy* CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, class FSlateWindowElementList& InDrawElements, class UEdGraph* InGraphObj) const;
	virtual void GetContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const override;
	virtual EGraphType GetGraphType(const UEdGraph* TestEdGraph) const { return GT_StateMachine; }
	virtual TSharedPtr<FEdGraphSchemaAction> GetCreateCommentAction() const override;
	// UEdSchema Interface //

};
