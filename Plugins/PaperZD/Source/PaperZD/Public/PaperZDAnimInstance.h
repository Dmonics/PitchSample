// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Templates/SubclassOf.h"
#include "PaperZDAnimInstance.generated.h"

class UPaperZDAnimSequence;
class UPaperZDAnimPlayer;
class UPaperZDAnimNode_Jump;

USTRUCT()
struct FAnimQueueInfo
{
	GENERATED_BODY()

	FAnimQueueInfo() :
		AnimSequence(nullptr)
		, bLooping(false)
		, bPopOnCompletion(false)
	{
	}

	FAnimQueueInfo(UPaperZDAnimSequence* InAnimSequence, bool InLooping = false, bool InPopOnCompletion = true) :
		 AnimSequence(InAnimSequence)
		, bLooping(InLooping)
		, bPopOnCompletion(InPopOnCompletion)
	{
	}

	UPROPERTY()
	UPaperZDAnimSequence* AnimSequence;

	UPROPERTY()
	bool bLooping;

	UPROPERTY()
	bool bPopOnCompletion;
};

UENUM()
namespace EPaperZDAnimationMode
{
	enum Type
	{
		Custom,
		StateMachine,
		Montage,
	};
}

UINTERFACE()
class PAPERZD_API UPaperZDAnimInstanceManager : public UInterface
{
	GENERATED_BODY()
};

class PAPERZD_API IPaperZDAnimInstanceManager : public IInterface
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent, Category = "AnimInstance Manager")
	void ConfigurePlayer(class UPaperZDAnimPlayer* Player);
	//virtual void GetWorld() {};
	//virtual void OnAnimationUpdated(FromSeq, ToSeq);
	//virtual void OnAnimSequenceEndReached(Seq);
};

UCLASS()
class PAPERZD_API UPaperZDAnimInstance : public UObject
{
	GENERATED_UCLASS_BODY()

private:
	UPROPERTY()
	TArray<FAnimQueueInfo>AnimationQueue;
	
	//Map for animation render components
	UPROPERTY()
	TMap<TSubclassOf<class UPaperZDAnimSequence>, class UPrimitiveComponent*> SequenceRenderComponents;
	
public:
	/* Root node is where the state machine starts, it must have a connection to a state. */
	UPROPERTY()
	class UPaperZDAnimNode_Root* RootNode;

	/* Redirectors are used for changing the flow of the state machine manually and can be indexed by name. */
	UPROPERTY()
	TMap<FName, UPaperZDAnimNode_Jump*> JumpNodeMap;

	//Current state of the machine
	UPROPERTY(Transient)
	class UPaperZDAnimNode* CurrentState;

	//PaperCharacter
	UPROPERTY(BlueprintReadOnly, Transient, Category="PaperZD")
	class APaperZDCharacter* PaperCharacter;

	UPROPERTY()
	TMap<FName,FName> NotifyFunctionMap;

	//The underlying animation mode
	EPaperZDAnimationMode::Type AnimationUpdateMode;

	/* If this AnimBP should globally ignore time dilation. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PaperZD")
	bool bIgnoreTimeDilation;

private:
	UPROPERTY(Transient)
	UPaperZDAnimPlayer* AnimPlayer;

	/**
	 * If we should enable Transitional States, so even after transitioning from State A to State B, we allow for more levels of recursions.
	 * This allows for a simpler more compact graph, by just connecting states and let them naturally flow to the end state, without seeing flicker as it's done on the same frame.
	 * Deactivating this makes each state become an endstate on each tick, and flow has to be done manually, but is overall faster.
	 */
	UPROPERTY(EditAnywhere, Category = "PaperZD")
	bool bAllowTransitionalStates;

public:
	/**
	* We obtain the world from the character defined
	*/
	virtual class UWorld* GetWorld() const override;

	virtual void Tick(float DeltaTime);

	/* Getter for transitional states */
	bool AllowsTransitionalStates() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "PaperZD")
	void OnTick(float DeltaTime, class APaperZDCharacter *Character);

	/* Init the AnimInstance using a Character (specialized case) */
	void Init(class APaperZDCharacter* Character);

	/* Init the AnimInstance using the general interface. */
	void Init(TScriptInterface<IPaperZDAnimInstanceManager> Manager);

	/* Specialized event called when initializing this AnimInstance using a PaperZDCharacter.*/
	UFUNCTION(BlueprintImplementableEvent, Category = "PaperZD")
	void OnInit(class APaperZDCharacter* Character);

	/**
	 * Changes current execution state and jumps to the given JumpNode name.
	 * @param JumpName		Name of the jump node we wish to go to.
	 */
	UFUNCTION(BlueprintCallable, Category = "PaperZD")
	void JumpToNode(FName JumpName);

	/* Adds a new animation on top of the stack. The animation queue is a LIFO stack. */
	UFUNCTION()
	void EnqueueAnimation(FAnimQueueInfo QueueInfo);

	/* Clears all the Animation Queue Stack. */
	UFUNCTION()
	void ClearAnimationQueue();

	/* Runs the animation queue, poping or updating the animation stack as needed*/
	void ProcessAnimationQueue(float DeltaTime);

	/* Obtains the current player, responsible of storing the playback information of this AnimInstance. */
	UFUNCTION(BlueprintPure, Category = "PaperZD|Playback")
	UPaperZDAnimPlayer* GetPlayer() const;

	/**
	 * Event called when we update playback, changing to a new sequence. 
	 * @param From				The previously played sequence
	 * @param To				The sequence that will be played now
	 * @param CurrentProgress	The progress in which the "From" sequence was before changing, ranging from [0-1]
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Playback")
	void OnAnimSequenceUpdated(const UPaperZDAnimSequence* From, const UPaperZDAnimSequence* To, float CurrentProgress);

	/**
	 * Called when an AnimSequence completes playback. Will only be called for non-looping sequences, as the looping sequences do not really "end" their playback.
	 * @param InAnimSequence	Sequence that reached its end
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Playback")
	void OnAnimSequencePlaybackComplete(const UPaperZDAnimSequence* InAnimSequence);

private:
	/* Obtains the equivalent delta time to use ignoring time dilation. */
	float GetDeltaTimeIgnoredDilation(float DeltaTime);

};
