// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "PaperZDAnimGraphSchema.h"
#include "PaperZDEditor.h"
#include "PaperZDAnimGraph.h"
#include "PaperZDAnimBP.h"
#include "Framework/Commands/GenericCommands.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

//Graph Nodes
#include "EdGraph/EdGraphNode.h"
#include "EdGraphNode_Comment.h"

#include "Nodes/PaperZDAnimGraphNode_Root.h"
#include "Nodes/PaperZDAnimGraphNode_Transition.h"
#include "Nodes/PaperZDAnimGraphNode_State.h"
#include "Nodes/PaperZDAnimGraphNode_Conduit.h"
#include "Nodes/PaperZDAnimGraphNode_Jump.h"

#include "AnimNodes/PaperZDAnimNode_Root.h"
#include "AnimNodes/PaperZDAnimNode_State.h"
#include "AnimNodes/PaperZDAnimNode_Transition.h"
#include "AnimNodes/PaperZDAnimNode_Conduit.h"
#include "AnimNodes/PaperZDAnimNode_Jump.h"

#include "PaperZDStateMachineConnectionDrawingPolicy.h"

//Editor
#include "ScopedTransaction.h"
#include "GraphEditorActions.h"
#include "ToolMenus.h"

//Nodes
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"


#define LOCTEXT_NAMESPACE "PaperZDAnimGraphSchema"

/////////////////////////////////////////////////////
// FPaperZDAnimGraphSchemaAction_NewNode

UEdGraphNode* FPaperZDAnimGraphSchemaAction_NewNode::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode/* = true*/)
{
	check(AnimNodeClass);

	UPaperZDAnimBP* AnimBP = CastChecked<UPaperZDAnimGraph>(ParentGraph)->GetAnimBP();
	const FScopedTransaction Transaction(LOCTEXT("PaperZDAnimBPEditorNewNode", "PaperZD AnimBP Editor: New Anim Node"));
	ParentGraph->Modify();
	AnimBP->Modify();
	//Construct the node 
	UPaperZDAnimNode *NewNode = AnimBP->ConstructAnimNode<UPaperZDAnimNode>(AnimNodeClass, bSelectNewNode);

	NewNode->GraphNode->NodePosX = Location.X;
	NewNode->GraphNode->NodePosY = Location.Y;

	//Create the autowire
	NewNode->GraphNode->AutowireNewNode(FromPin);

	AnimBP->PostEditChange();
	AnimBP->MarkPackageDirty();

	//Mark animbp as modified
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(AnimBP);

	return NewNode->GraphNode;
}

/////////////////////////////////////////////////////
// FPaperZDAnimGraphSchemaAction_NewComment

UEdGraphNode* FPaperZDAnimGraphSchemaAction_NewComment::PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
{
	// Add menu item for creating comment boxes
	UEdGraphNode_Comment* CommentTemplate = NewObject<UEdGraphNode_Comment>();

	UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForGraph(ParentGraph);

	FVector2D SpawnLocation = Location;

	FSlateRect Bounds;
	if ((Blueprint != nullptr) && FKismetEditorUtilities::GetBoundsForSelectedNodes(Blueprint, Bounds, 50.0f))
	{
		CommentTemplate->SetBounds(Bounds);
		SpawnLocation.X = CommentTemplate->NodePosX;
		SpawnLocation.Y = CommentTemplate->NodePosY;
	}

	UEdGraphNode* NewNode = FEdGraphSchemaAction_NewNode::SpawnNodeFromTemplate<UEdGraphNode_Comment>(ParentGraph, CommentTemplate, SpawnLocation, bSelectNewNode);

	// Update Analytics for these nodes
	FBlueprintEditorUtils::AnalyticsTrackNewNode(NewNode);

	// Mark Blueprint as structurally modified since
	// UK2Node_Comment::NodeCausesStructuralBlueprintChange used to return true
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);

	return NewNode;
}

//////////////////////////////////////////////////////////////////////////
//// PaperZD Anim Graph Schema
//////////////////////////////////////////////////////////////////////////
UPaperZDAnimGraphSchema::UPaperZDAnimGraphSchema(const FObjectInitializer& ObjectInitializer)
	: Super()
{
}

FConnectionDrawingPolicy* UPaperZDAnimGraphSchema::CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj) const
{
	return new FPaperZDStateMachineConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements, InGraphObj);
}

void UPaperZDAnimGraphSchema::CreateDefaultNodesForGraph(UEdGraph& Graph) const
{
	//Default graph node has to be created without an action, because actions have UNDOs registered on them, and we don't want the Root node to be undone by accident
	UPaperZDAnimBP* AnimBP = CastChecked<UPaperZDAnimGraph>(&Graph)->GetAnimBP();
	UPaperZDAnimNode *AnimNode = AnimBP->ConstructAnimNode<UPaperZDAnimNode>(UPaperZDAnimNode_Root::StaticClass(), false);

	SetNodeMetaData(AnimNode->GraphNode, FNodeMetadata::DefaultGraphNode);
}
	
void UPaperZDAnimGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	//Anim State Action
	TSharedPtr<FPaperZDAnimGraphSchemaAction_NewNode> AddStateAction(new FPaperZDAnimGraphSchemaAction_NewNode(
		LOCTEXT("PaperZDAnimNodeCategory", "Nodes"),
		LOCTEXT("PaperZDAnimNodeState","Animation State"), 
		LOCTEXT("NewPaperZDAnimNodeTooltipState", "Adds an Animation Node node here"),
		0));
	AddStateAction->AnimNodeClass = UPaperZDAnimNode_State::StaticClass();
	ContextMenuBuilder.AddAction(AddStateAction);

	TSharedPtr<FPaperZDAnimGraphSchemaAction_NewNode> AddConduitAction(new FPaperZDAnimGraphSchemaAction_NewNode(
		LOCTEXT("PaperZDAnimNodeCategory", "Nodes"),
		LOCTEXT("PaperZDAnimNodeConduit", "Conduit"),
		LOCTEXT("NewPaperZDAnimNodeTooltipConduit", "Adds a Conduit node here"),
		0));
	AddConduitAction->AnimNodeClass = UPaperZDAnimNode_Conduit::StaticClass();
	ContextMenuBuilder.AddAction(AddConduitAction);

	TSharedPtr<FPaperZDAnimGraphSchemaAction_NewNode> AddJumpAction(new FPaperZDAnimGraphSchemaAction_NewNode(
		LOCTEXT("PaperZDAnimNodeCategory", "Nodes"),
		LOCTEXT("PaperZDAnimNodeJump", "Jump"),
		LOCTEXT("NewPaperZDAnimNodeTooltipJump", "Adds a Jump node here"),
		0));
	AddJumpAction->AnimNodeClass = UPaperZDAnimNode_Jump::StaticClass();
	ContextMenuBuilder.AddAction(AddJumpAction);
	
	TSharedPtr<FPaperZDAnimGraphSchemaAction_NewComment> AddCommentAction(new FPaperZDAnimGraphSchemaAction_NewComment(
		LOCTEXT("PaperZDAnimNodeCategory", "Nodes"),
		LOCTEXT("PaperZDAnimNodeComment", "Comment"),
		LOCTEXT("NewPaperZDAnimNodeTooltipComment", "Adds a Comment Node node here"),
		0));
	ContextMenuBuilder.AddAction(AddCommentAction);
}

void UPaperZDAnimGraphSchema::GetContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	if (Context && Context->Node)
	{
		UBlueprint* OwnerBlueprint = FBlueprintEditorUtils::FindBlueprintForGraphChecked(Context->Graph);
		FToolMenuSection& Section = Menu->AddSection("ZDAnimationStateMachineNodeActions", LOCTEXT("NodeActionsMenuHeader", "Node Actions"));
		if (!Context->bIsDebugging)
		{
			// Node contextual actions
			Section.AddMenuEntry(FGenericCommands::Get().Delete);
			Section.AddMenuEntry(FGenericCommands::Get().Cut);
			Section.AddMenuEntry(FGenericCommands::Get().Copy);
			Section.AddMenuEntry(FGenericCommands::Get().Duplicate);
			Section.AddMenuEntry(FGraphEditorCommands::Get().ReconstructNodes);
			Section.AddMenuEntry(FGraphEditorCommands::Get().BreakNodeLinks);
			if (Context->Node->bCanRenameNode)
			{
				Section.AddMenuEntry(FGenericCommands::Get().Rename);
			}
		}
	}

	Super::GetContextMenuActions(Menu, Context);
}

const FPinConnectionResponse UPaperZDAnimGraphSchema::CanCreateConnection(const UEdGraphPin* PinA, const UEdGraphPin* PinB) const
{
	if (!PinA || !PinB)
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("No entry pin!"));

	UPaperZDAnimGraphNode *NodeA = Cast<UPaperZDAnimGraphNode>(PinA->GetOwningNode());
	UPaperZDAnimGraphNode *NodeB = Cast<UPaperZDAnimGraphNode>(PinB->GetOwningNode());

	//Abort on same nodes
	if (NodeA == NodeB)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Cannot connect to the same node"));
	}

	//Create a transition for state nodes
	if (NodeA->IsA(UPaperZDAnimGraphNode_State::StaticClass()) && NodeB->IsA(UPaperZDAnimGraphNode_State::StaticClass()))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_MAKE_WITH_CONVERSION_NODE, TEXT("Create a Transition"));
	}
	
	//Don't allow transition-transition connections
	if (NodeA->IsA(UPaperZDAnimGraphNode_Transition::StaticClass()) && NodeB->IsA(UPaperZDAnimGraphNode_Transition::StaticClass()))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Cannot connect transition to transition"));
	}

	//Create Transition - State Connection
	if (NodeA->IsA(UPaperZDAnimGraphNode_Transition::StaticClass()) && NodeB->IsA(UPaperZDAnimGraphNode_State::StaticClass()))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_A, TEXT("Connect transition to State"));
	}

	//Create State - Conduit
	if (NodeA->IsA(UPaperZDAnimGraphNode_State::StaticClass()) && NodeB->IsA(UPaperZDAnimGraphNode_Conduit::StaticClass()))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_MAKE_WITH_CONVERSION_NODE, TEXT("Create a Transition"));
	}

	//Create Conduit - State
	if (NodeA->IsA(UPaperZDAnimGraphNode_Conduit::StaticClass()) && NodeB->IsA(UPaperZDAnimGraphNode_State::StaticClass()))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_MAKE_WITH_CONVERSION_NODE, TEXT("Create a Transition"));
	}

	//Create Conduit - Conduit
	if (NodeA->IsA(UPaperZDAnimGraphNode_Conduit::StaticClass()) && NodeB->IsA(UPaperZDAnimGraphNode_Conduit::StaticClass()))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_MAKE_WITH_CONVERSION_NODE, TEXT("Create a Transition"));
	}

	//Create State - Transition Connection
	if (NodeA->IsA(UPaperZDAnimGraphNode_State::StaticClass()) && NodeB->IsA(UPaperZDAnimGraphNode_Transition::StaticClass()))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_B, TEXT("Connect State to Transition"));
	}

	//Create Root - State Connection
	if (NodeA->IsA(UPaperZDAnimGraphNode_Root::StaticClass()) && NodeB->IsA(UPaperZDAnimGraphNode_State::StaticClass()))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_A, TEXT("Connect Root to State"));
	}

	//Create Jump - State
	if (NodeA->IsA(UPaperZDAnimGraphNode_Jump::StaticClass()) && NodeB->IsA(UPaperZDAnimGraphNode_State::StaticClass()))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_A, TEXT("Connect Jump to State"));
	}

	return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Connection not Allowed"));
}

bool UPaperZDAnimGraphSchema::TryCreateConnection(UEdGraphPin* PinA, UEdGraphPin* PinB) const
{
	//Transform an OUTPUT - OUTPUT connection into an INPUT - OUTPUT Connection (SPaperZDAnimGraphNode_State has only output as a draggable node, input node is invisible)
	if (PinB->Direction == PinA->Direction)
	{
		if (UPaperZDAnimGraphNode* Node = Cast<UPaperZDAnimGraphNode>(PinB->GetOwningNode()))
		{
			if (PinA->Direction == EGPD_Input)
			{
				PinB = Node->GetOutputPin();
			}
			else
			{
				PinB = Node->GetInputPin();
			}
		}
	}

	const bool bModified = UEdGraphSchema::TryCreateConnection(PinA, PinB);

	if (bModified)
	{
		UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForNodeChecked(PinA->GetOwningNode());
		FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
	}

	return bModified;
}

bool UPaperZDAnimGraphSchema::CreateAutomaticConversionNodeAndConnections(UEdGraphPin* PinA, UEdGraphPin* PinB) const
{
	UPaperZDAnimGraphNode *NodeA = Cast<UPaperZDAnimGraphNode>(PinA->GetOwningNode());
	UPaperZDAnimGraphNode *NodeB = Cast<UPaperZDAnimGraphNode>(PinB->GetOwningNode());

	if ((NodeA->IsA(UPaperZDAnimGraphNode_State::StaticClass()) || NodeA->IsA(UPaperZDAnimGraphNode_Conduit::StaticClass())) && (NodeB->IsA(UPaperZDAnimGraphNode_State::StaticClass()) || NodeB->IsA(UPaperZDAnimGraphNode_Conduit::StaticClass())))
	{
		UPaperZDAnimGraphNode_Transition *TransitionNode = FPaperZDAnimGraphSchemaAction_NewNode::SpawnGraphNodeFromAnimNode<UPaperZDAnimGraphNode_Transition>(NodeA->GetGraph(), UPaperZDAnimNode_Transition::StaticClass(), FVector2D(0.0f, 0.0f), false);

		if (PinA->Direction == EEdGraphPinDirection::EGPD_Output)
			TransitionNode->CreateConnections(NodeA, NodeB);
		else
			TransitionNode->CreateConnections(NodeB, NodeA);

		return true;
	}

	return false;
}

void UPaperZDAnimGraphSchema::GetGraphDisplayInformation(const UEdGraph & Graph, FGraphDisplayInfo & DisplayInfo) const
{
	DisplayInfo.DisplayName = FText::FromString(TEXT("AnimationGraph"));
	DisplayInfo.PlainName = FText::FromString(TEXT("Animation Graph"));
	DisplayInfo.Tooltip = FText::FromString(TEXT("The Animation tree of this blueprint"));
}

TSharedPtr<FEdGraphSchemaAction> UPaperZDAnimGraphSchema::GetCreateCommentAction() const
{
	return TSharedPtr<FPaperZDAnimGraphSchemaAction_NewComment>(new FPaperZDAnimGraphSchemaAction_NewComment);
}

#undef LOCTEXT_NAMESPACE
