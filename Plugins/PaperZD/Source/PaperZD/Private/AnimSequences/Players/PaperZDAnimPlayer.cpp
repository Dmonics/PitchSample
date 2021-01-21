// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "AnimSequences/Players/PaperZDAnimPlayer.h"
#include "AnimSequences/PaperZDAnimSequence.h"
#include "Components/PrimitiveComponent.h"

//Notifies
#include "Notifies/PaperZDAnimNotify_Base.h"

UPaperZDAnimPlayer::UPaperZDAnimPlayer() : Super()
{
	bPlaying = true;
	CurrentPlaybackTime = 0.0f;
	BufferPlaybackTime = 0.0f;
	PlaybackMode = EAnimPlayerPlaybackMode::Forward;
	bPreviewPlayer = false;
}

float UPaperZDAnimPlayer::GetCurrentPlaybackTime() const
{
	return BufferPlaybackTime;
}

float UPaperZDAnimPlayer::GetPlaybackProgress() const
{
	return LastPlayedSequence.IsValid() && LastPlayedSequence->GetTotalDuration() > 0.0f ? BufferPlaybackTime / LastPlayedSequence->GetTotalDuration() : 0.0f;
}

void UPaperZDAnimPlayer::SetCurrentPlaybackTime(float InCurrentTime)
{
	CurrentPlaybackTime = InCurrentTime;
	BufferPlaybackTime = CurrentPlaybackTime;
}

void UPaperZDAnimPlayer::Reset()
{
	CurrentPlaybackTime = 0.0f;
	BufferPlaybackTime = CurrentPlaybackTime;
	bNotifyCompletion = true;
}

void UPaperZDAnimPlayer::ClearPlaybackBuffer()
{
	BufferPlaybackTime = 0.0f;
}

void UPaperZDAnimPlayer::ResumePlayback()
{
	bPlaying = true;
}

void UPaperZDAnimPlayer::PausePlayback()
{
	bPlaying = false;
}

void UPaperZDAnimPlayer::SetIsPreviewPlayer(bool bInPreviewPlayer)
{
	bPreviewPlayer = bInPreviewPlayer;
}

void UPaperZDAnimPlayer::SequencePlaybackChanged(const UPaperZDAnimSequence* AnimSequence, bool bLooping)
{
	//Store previous values
	TWeakObjectPtr<const UPaperZDAnimSequence> PreviousSequence = LastPlayedSequence;
	const float PreviousPlaybackTime = CurrentPlaybackTime;

	//Reset playback parameters
	LastPlayedSequence = AnimSequence;
	Reset();

	//Broadcast and communicate the change of sequence (Only if we do have a previously valid sequence though)
	if (PreviousSequence.IsValid())
	{
		const float CurrentProgress = PreviousPlaybackTime / PreviousSequence->GetTotalDuration();
		OnPlaybackSequenceChanged.Broadcast(PreviousSequence.Get(), AnimSequence, CurrentProgress);
	}

	//Find the render component for this class, if it exists
	if (UPrimitiveComponent** pRenderComp = RenderComponentMap.Find(AnimSequence->GetClass()))
	{
		AnimSequence->BeginSequencePlayback(*pRenderComp, bLooping, bPreviewPlayer);
	}
}

//Playback controls
void UPaperZDAnimPlayer::TickPlayback(const UPaperZDAnimSequence* AnimSequence, const float DeltaTime, const bool bLooping, class UPaperZDAnimInstance *OwningInstance)
{
	if (AnimSequence && bPlaying)
	{
		float currentTime;
		bool bSequencePlaybackComplete = false;
		//Time resets when playing a new sequence
		if (!LastPlayedSequence.IsValid() || LastPlayedSequence.Get() != AnimSequence)
		{
			SequencePlaybackChanged(AnimSequence, bLooping);
			currentTime = CurrentPlaybackTime;
		}
		else
		{
			//We are running on the same AnimSequence as before, increment time normally
			if (PlaybackMode == EAnimPlayerPlaybackMode::Forward)
			{
				currentTime = CurrentPlaybackTime + DeltaTime;
				bSequencePlaybackComplete = currentTime > AnimSequence->GetTotalDuration();
				currentTime = bLooping && bSequencePlaybackComplete ? FMath::Max(AnimSequence->GetTotalDuration() - currentTime, 0.0f) : FMath::Min(currentTime, AnimSequence->GetTotalDuration());

			}
			else
			{
				currentTime = CurrentPlaybackTime - DeltaTime;
				bSequencePlaybackComplete = currentTime < 0.0f;
				currentTime = bLooping && bSequencePlaybackComplete ? AnimSequence->GetTotalDuration() + currentTime : FMath::Max(currentTime, 0.0f);
			}
		}
		
		//This sounds like a redundant thing to do, but sequencer cannot TICK the same way as the game, so this method works for both worlds... even if the TICK has to be re calculated inside
		SetSequencePlaybackTime(AnimSequence, currentTime, true, OwningInstance, bLooping);

		//Notify anyone who needs to know about looping or sequence playback completion
		if (bSequencePlaybackComplete && bNotifyCompletion)
		{
			//We separate the delegates into two, one for looping, and one for playback completion
			//We need to call this methods after the SetSequencePlaybackTime, so the state and render get the chance to update
			if (bLooping)
			{
				OnPlaybackSequenceLooped.Broadcast(AnimSequence);
			}
			else
			{
				OnPlaybackSequenceComplete.Broadcast(AnimSequence);

				//In order to avoid calling this every frame after reaching its end we setup this flag
				bNotifyCompletion = false;
			}
		}
	}
}

void UPaperZDAnimPlayer::SetSequencePlaybackTime(const UPaperZDAnimSequence* AnimSequence, float Time, const bool bFireNotifies, class UPaperZDAnimInstance* OwningInstance /* = nullptr */, bool bLooping /* = false */)
{
	if (AnimSequence)
	{
		//Make sure we haven't updated the sequence, this method can be called without Tick's intervention
		if (!LastPlayedSequence.IsValid() || LastPlayedSequence.Get() != AnimSequence)
		{
			SequencePlaybackChanged(AnimSequence, bLooping);
		}

		//Find the render component for this class, if it exists
		if (UPrimitiveComponent** pRenderComp = RenderComponentMap.Find(AnimSequence->GetClass()))
		{
			AnimSequence->UpdateRenderPlayback(*pRenderComp, Time, bPreviewPlayer);

			//Tick the notifies
			if (bFireNotifies)
			{
				for (UPaperZDAnimNotify_Base* Notify : AnimSequence->GetAnimNotifies())
				{
					Notify->TickNotify(Time - CurrentPlaybackTime, Time, CurrentPlaybackTime, *pRenderComp, OwningInstance);
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Trying to play AnimSequence '%s' without having registered a PrimitiveComponent as Renderer on AnimPlayer '%s'"), *AnimSequence->GetName(), *GetName());
		}

		//Update the current playback time
		CurrentPlaybackTime = Time;
		BufferPlaybackTime = CurrentPlaybackTime;
	}
}

//Render component methods
void UPaperZDAnimPlayer::RegisterRenderComponent(TSubclassOf<UPaperZDAnimSequence> AnimSequenceClass, UPrimitiveComponent* RenderComponent)
{
	RenderComponentMap.Add(AnimSequenceClass, RenderComponent);

	//Chance to configure the given render component using the SequenceClass
	UPaperZDAnimSequence* AnimSequenceCDO = AnimSequenceClass->GetDefaultObject<UPaperZDAnimSequence>();
	AnimSequenceCDO->ConfigureRenderComponent(RenderComponent, bPreviewPlayer);
}