// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once
#include "EdGraph/EdGraphNode.h"
#include "PaperZDAnimInstance.h"
#include "PaperZDAnimNode.generated.h"

namespace EPaperZDNodeType {
	UENUM()
		enum NodeType : uint8 {
		NODE_Root UMETA(DisplayName = "Root Node"),
		NODE_State UMETA(DisplayName = "State Node"),
		NODE_Transition UMETA(DisplayName = "Transition Node"),
		NODE_Conduit UMETA(DisplayName = "Conduit Node"),
		NODE_Jump UMETA(DisplayName = "Jump Node")
	};
}

UCLASS(abstract)
class PAPERZD_API UPaperZDAnimNode : public UObject
{
	GENERATED_UCLASS_BODY()

public:
#if WITH_EDITORONLY_DATA
	UPROPERTY()
	UEdGraphNode *GraphNode;
#endif
	//Per class Type, doesn't need to be serialized
	EPaperZDNodeType::NodeType Type;

	//The anim instance that owns this node
	UPaperZDAnimInstance *AnimInstance;

public:
#if WITH_EDITOR
	//Gets all the UProperties marked as meta=ShowAsNodePin
	TArray<UProperty *> GetPinProperties();
#endif

	//Process the state and modify the owning instance when necessary. The Current State expects to be changed on this method when it applies
	virtual void Tick(float DeltaTime, UPaperZDAnimInstance* OwningInstance) {};
	virtual void ConnectOutputWith(TArray<UPaperZDAnimNode*>PossibleNodes) {};
	virtual TArray<UPaperZDAnimNode *> GetOutputNodes() const { return TArray<UPaperZDAnimNode*>(); };
	virtual void Init(UPaperZDAnimInstance* OwningInstance) {};
	void PropagateInit(UPaperZDAnimInstance* OwningInstance, TSet<UPaperZDAnimNode*>& VisitedNodes);

	//Called when state machine tries "enters" the node, VisitedNodes given to avoid possible circular recursions
	virtual void Enter(UPaperZDAnimInstance* OwningInstance) {}
	virtual bool CanEnter(UPaperZDAnimInstance* OwningInstance, TSet<const UPaperZDAnimNode*>& VisitedNodes) const;
};
