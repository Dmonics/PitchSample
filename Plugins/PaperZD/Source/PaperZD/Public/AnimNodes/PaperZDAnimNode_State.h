// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once
#include "AnimNodes/PaperZDAnimNode.h"
#include "PaperZDAnimNode_State.generated.h"

class UPaperZDAnimSequence;
class UPaperZDAnimTrack;
class UPaperZDAnimNotify_Base;
UCLASS()
class PAPERZD_API UPaperZDAnimNode_State : public UPaperZDAnimNode
{
	GENERATED_UCLASS_BODY()

#if WITH_EDITOR
		//Added for version update to AnimSequence only
		friend class FPaperZDRuntimeEditorProxy;
#endif

public:
	UPROPERTY()
	TArray<class UPaperZDAnimNode_Transition*> Transitions;

	UPROPERTY()
	bool bShouldLoop;
	
	UPROPERTY()
	UPaperZDAnimSequence* AnimSequence;

protected:
	UPROPERTY()
	TArray<UPaperZDAnimNotify_Base*>AnimNotifies_DEPRECATED;

private:
	UPROPERTY()
	TArray<UPaperZDAnimTrack *> CachedAnimTracks_DEPRECATED; //Cache of AnimTracks to avoid GC

	/* Maximum number of recursions to allow when allowing transitional states */
	static const int32 MaxTransitionalStateRecursion;

public:
	virtual void Tick(float DeltaTime, UPaperZDAnimInstance *OwningInstance) override;
	virtual void ConnectOutputWith(TArray<UPaperZDAnimNode *>PossibleNodes) override;
	virtual TArray<UPaperZDAnimNode *> GetOutputNodes() const override;
	virtual void Init(UPaperZDAnimInstance *OwningInstance) override;
	virtual void Enter(UPaperZDAnimInstance *OwningInstance) override;

private:
	/**
	 * Process transition nodes and handles transitional states (if enabled). Limits internal recursion to MaxTransitionalStateRecursion and avoids StackOverflow by halting
	 * transitional updates when reaching the maximum recursion number. Avoids circular recursion by calling CanEnter for testing transitions
	 */
	void ProcessTransitions(UPaperZDAnimInstance* OwningInstance, TSet<const UPaperZDAnimNode*> VisitedNodes, int32 NumRecursions = 0);
};
