// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "PaperZDAnimInstance.h"
#include "PaperZD.h"
#include "PaperZDCharacter.h"
#include "AnimSequences/PaperZDAnimSequence.h"
#include "AnimNodes/PaperZDAnimNode_Root.h"
#include "AnimNodes/PaperZDAnimNode_State.h"
#include "AnimNodes/PaperZDAnimNode_Jump.h"
#include "AnimSequences/Players/PaperZDAnimPlayer.h"

//#include "CoreUObject.h"

//For time dilation
#include "Kismet/GameplayStatics.h"

UPaperZDAnimInstance::UPaperZDAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(), AnimationUpdateMode(EPaperZDAnimationMode::StateMachine)
{
	//Setup CDO values
	bIgnoreTimeDilation = false;
	bAllowTransitionalStates = true;
}

UWorld* UPaperZDAnimInstance::GetWorld() const
{
	return PaperCharacter ? PaperCharacter->GetWorld() : NULL;
}

UPaperZDAnimPlayer* UPaperZDAnimInstance::GetPlayer() const
{
	return AnimPlayer;
}

bool UPaperZDAnimInstance::AllowsTransitionalStates() const
{
	return bAllowTransitionalStates;
}

void UPaperZDAnimInstance::Tick(float DeltaTime)
{
	if (bIgnoreTimeDilation)
	{
		//Modify the DeltaTime to use an non-dilated value
		DeltaTime = GetDeltaTimeIgnoredDilation(DeltaTime);
	}

	//Tick the state, so it has an opportunity to change the animation queue
	if (ensure(CurrentState) && AnimationUpdateMode == EPaperZDAnimationMode::StateMachine)
	{
		CurrentState->Tick(DeltaTime, this);
	}

	//Apply animation queue updates
	if (AnimationUpdateMode != EPaperZDAnimationMode::Custom)
	{
		ProcessAnimationQueue(DeltaTime);
	}

	//Call the blueprint method, if any
	OnTick(DeltaTime, PaperCharacter);
}

void UPaperZDAnimInstance::Init(APaperZDCharacter* Character)
{
	//Init with the manager
	Init(TScriptInterface<IPaperZDAnimInstanceManager>(Character));

	//Character only calls
	PaperCharacter = Character;
	OnInit(Character);
}

void UPaperZDAnimInstance::Init(TScriptInterface<IPaperZDAnimInstanceManager> Manager)
{
	//Init the player
	AnimPlayer = NewObject<UPaperZDAnimPlayer>(this);
	IPaperZDAnimInstanceManager::Execute_ConfigurePlayer(Manager.GetObject(), AnimPlayer);

	//Bind the corresponding delegates
	AnimPlayer->OnPlaybackSequenceChanged.AddDynamic(this, &UPaperZDAnimInstance::OnAnimSequenceUpdated);
	AnimPlayer->OnPlaybackSequenceComplete.AddDynamic(this, &UPaperZDAnimInstance::OnAnimSequencePlaybackComplete);

	//Init every node, starting from the RootNode
	CurrentState = RootNode;
	TSet<UPaperZDAnimNode*> VisitedNodes;
	RootNode->PropagateInit(this, VisitedNodes);

	//Continue from the JumpNodes, as they can be unbound from the root node
	for (const auto& KeyValuePair : JumpNodeMap)
	{
		KeyValuePair.Value->PropagateInit(this, VisitedNodes);
	}
}

void UPaperZDAnimInstance::JumpToNode(FName Name)
{
	UPaperZDAnimNode_Jump** pJumpNode = JumpNodeMap.Find(Name);
	if (pJumpNode)
	{
		(*pJumpNode)->Enter(this);
	}
}

/************************************************************************/
/*     Animation Queue Related                                          */
/************************************************************************/
void UPaperZDAnimInstance::EnqueueAnimation(FAnimQueueInfo QueueInfo)
{
	if (QueueInfo.AnimSequence)
	{
		AnimationQueue.Insert(QueueInfo, 0);
	}
}


void UPaperZDAnimInstance::ClearAnimationQueue()
{
	AnimationQueue.Empty();

	//Queue has been cleared, animations currently running on the player will most likely be replaced, so the playback time is invalid now
	AnimPlayer->ClearPlaybackBuffer();	
}

void UPaperZDAnimInstance::ProcessAnimationQueue(float DeltaTime)
{
	//Early out
	if (!AnimationQueue.Num())
		return;

	//Obtain the corresponding info and the sequence
	FAnimQueueInfo& Info = AnimationQueue.Last();

	//Actually play the animation
	AnimPlayer->TickPlayback(Info.AnimSequence, DeltaTime, Info.bLooping, this);

	//Playtime's over kid
	if (Info.bPopOnCompletion && AnimPlayer->GetCurrentPlaybackTime() >= Info.AnimSequence->GetTotalDuration())
	{
		AnimationQueue.Pop();
	}
}

float UPaperZDAnimInstance::GetDeltaTimeIgnoredDilation(float DeltaTime)
{
	const float timeDilation = UGameplayStatics::GetGlobalTimeDilation(this);
	return DeltaTime / timeDilation;
}