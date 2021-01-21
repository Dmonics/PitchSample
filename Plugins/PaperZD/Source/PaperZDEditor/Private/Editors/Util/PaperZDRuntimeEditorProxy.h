// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PaperZDAnimBP.h"

class UEdGraph;
class UPaperZDAnimNode;
class UPaperZDAnimGraphNode;

/**
 * Proxy class used to make Editor related changes that need to be called from the Runtime module of this plugin.
 * Normally called by the Animation Blueprint to create nodes, update versions and handle AnimSequence duplication
 */
class FPaperZDRuntimeEditorProxy : public IPaperZDEditorProxy
{
public:
	FPaperZDRuntimeEditorProxy() {}
	~FPaperZDRuntimeEditorProxy() {}

	//Registers an instance of the RuntimeEditorProxy with the AnimBP class
	static void Register();

	// ~IPaperZDEditorProxy
	virtual UEdGraph* CreateNewAnimationGraph(UPaperZDAnimBP* InAnimBP) override;
	virtual void SetupAnimNode(UEdGraph* AnimGraph, UPaperZDAnimNode *InAnimNode, bool bSelectNewNode) override;
	virtual void HandleDuplicateAnimBP(UPaperZDAnimBP* InAnimBP) override;
	virtual void UpdateVersionToAnimSequences(UPaperZDAnimBP *InAnimBP) override;
	virtual void UpdateVersionToAnimNodeOuterFix(UPaperZDAnimBP *InAnimBP) override;
	virtual void UpdateVersionToAnimSequenceCategoryAdded(UPaperZDAnimBP *InAnimBP) override;
	virtual void UpdateVersionToAnimSequenceAsStandaloneAsset(UPaperZDAnimBP* InAnimBP) override;
	// ~IPaperZDEditorProxy

private:
	UPaperZDAnimGraphNode* CreateRootNode(UEdGraph* AnimGraph, UPaperZDAnimNode *InAnimNode, bool bSelectNewNode);
	UPaperZDAnimGraphNode* CreateStateNode(UEdGraph* AnimGraph, UPaperZDAnimNode *InAnimNode, bool bSelectNewNode);
	UPaperZDAnimGraphNode* CreateTransitionNode(UEdGraph* AnimGraph, UPaperZDAnimNode *InAnimNode, bool bSelectNewNode);
	UPaperZDAnimGraphNode* CreateConduitNode(UEdGraph* AnimGraph, UPaperZDAnimNode *InAnimNode, bool bSelectNewNode);
	UPaperZDAnimGraphNode* CreateJumpNode(UEdGraph* AnimGraph, UPaperZDAnimNode *InAnimNode, bool bSelectNewNode);
};

