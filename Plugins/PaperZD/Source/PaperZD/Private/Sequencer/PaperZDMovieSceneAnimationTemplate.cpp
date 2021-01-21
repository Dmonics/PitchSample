// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "Sequencer/PaperZDMovieSceneAnimationTemplate.h"
#include "PaperZD.h"
#include "IMovieScenePlayer.h"
#include "PaperZDCharacter.h"
#include "Sequencer/PaperZDMovieSceneAnimationSection.h"
#include "AnimSequences/Players/PaperZDAnimPlayer.h"
#include "Engine/World.h"

#if WITH_EDITOR
//For backwards support
#include "PaperZDAnimBP.h"
#endif

namespace FPaperZDMovieSceneHelpers
{
	bool ShouldUsePreviewPlayback(IMovieScenePlayer& Player, UObject& RuntimeObject)
	{
		// we also use PreviewSetAnimPosition in PIE when not playing, as we can preview in PIE
		bool bIsNotInPIEOrNotPlaying = (RuntimeObject.GetWorld() && !RuntimeObject.GetWorld()->HasBegunPlay()) || Player.GetPlaybackStatus() != EMovieScenePlayerStatus::Playing;
		return GIsEditor && bIsNotInPIEOrNotPlaying;
	}
}

//////////////////////////////////////////////////////////////////////////
//// FPaperZDMovieSceneAnimationSectionTemplateParams
//////////////////////////////////////////////////////////////////////////

float FPaperZDMovieSceneAnimationSectionTemplateParams::MapTimeToAnimation(FFrameTime InPosition, FFrameRate InFrameRate, UPaperZDAnimSequence *Sequence) const
{
	InPosition = FMath::Clamp(InPosition, FFrameTime(SectionStartTime), FFrameTime(SectionEndTime - 1));

	const float SectionPlayRate = PlayRate;
	const float AnimPlayRate = FMath::IsNearlyZero(SectionPlayRate) ? 1.0f : SectionPlayRate;

	const float SeqLength = GetSequenceLength() - (StartOffset + EndOffset);

	float AnimPosition = FFrameTime::FromDecimal((InPosition - SectionStartTime).AsDecimal() * AnimPlayRate) / InFrameRate;
	if (SeqLength > 0.f)
	{
		AnimPosition = FMath::Fmod(AnimPosition, SeqLength);
	}
	AnimPosition += StartOffset;
	if (bReverse)
	{
		AnimPosition = (SeqLength - (AnimPosition - StartOffset)) + StartOffset;
	}

	return AnimPosition;
}

float FPaperZDMovieSceneAnimationSectionTemplateParams::MapTimeToWeight(FFrameTime InPosition, FFrameRate InFrameRate) const
{
	return InPosition > SectionStartTime && InPosition < SectionEndTime ? 1.0f : 0.0f;
}

//////////////////////////////////////////////////////////////////////////
//// FPaperZDMovieSceneAnimationTemplate
//////////////////////////////////////////////////////////////////////////
struct FMinimalAnimParameters
{
	FMinimalAnimParameters(UPaperZDAnimSequence *InSequence, float InEvalTime, uint32 InSectionId, float InWeight)
		: SequencePtr(InSequence)
		, EvalTime(InEvalTime)
		, Weight(InWeight)
		, SectionId(InSectionId)
	{}

	TWeakObjectPtr<UPaperZDAnimSequence> SequencePtr;
	float EvalTime;
	float Weight;
	uint32 SectionId;
};

struct FPaperZDPreAnimatedAnimationTokenProducer : IMovieScenePreAnimatedTokenProducer
{
	virtual IMovieScenePreAnimatedTokenPtr CacheExistingState(UObject& Object) const
	{
		struct FToken : IMovieScenePreAnimatedToken
		{
			FToken(APaperZDCharacter* InCharacter)
			{
				InCharacter->PrepareForMovieSequence();
			}

			virtual void RestoreState(UObject& ObjectToRestore, IMovieScenePlayer& Player)
			{
				CastChecked<APaperZDCharacter>(&ObjectToRestore)->RestoreFromMovieSequence();
			}
		};

		return FToken(CastChecked<APaperZDCharacter>(&Object));
	}
};

struct FPaperZDAnimationExecutionToken : IMovieSceneExecutionToken
{

private:
	FMinimalAnimParameters AnimParams;

public:
	FPaperZDAnimationExecutionToken(FMinimalAnimParameters InAnimParams) : AnimParams(InAnimParams) {};

	virtual void Execute(const FMovieSceneContext& Context, const FMovieSceneEvaluationOperand& Operand, FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) override
	{
		for (TWeakObjectPtr<> Object : Player.FindBoundObjects(Operand))
		{
			UObject* ObjectPtr = Object.Get();
			if (!ObjectPtr)
			{
				continue;
			}

			// Check for the zd character
			if (APaperZDCharacter* Character = Cast<APaperZDCharacter>(ObjectPtr))
			{
				// Save preanimated state here rather than the shared track as the shared track will still be evaluated even if there are no animations happening
				Player.SavePreAnimatedState(*(Character), TMovieSceneAnimTypeID<FPaperZDAnimationExecutionToken>(), FPaperZDPreAnimatedAnimationTokenProducer());
				
				UPaperZDAnimInstance* AnimInstance = Character->GetOrCreateAnimInstance();
#if WITH_EDITOR
				//@PATCH: For some reason, the interface methods WON'T get called when playing this on the preview editor, this here is just a patch to make sure that at least
				//the c++ method gets called, will not work for AnimBP methods though (as we cannot call the ConfigurePlayer method directly, due to the Interface assert)
				Character->ConfigurePlayer_Implementation(AnimInstance->GetPlayer()); 
#endif
				if (AnimInstance && AnimParams.SequencePtr.IsValid())
				{
					static const bool bLooping = true; //Sequencer animations loop by default
					const bool bPreviewPlayback = FPaperZDMovieSceneHelpers::ShouldUsePreviewPlayback(Player, *Character);

					const EMovieScenePlayerStatus::Type PlayerStatus = Player.GetPlaybackStatus();

					// If the playback status is jumping, ie. one such occurrence is setting the time for thumbnail generation, disable anim notifies updates because it could fire audio
					const bool bFireNotifies = !bPreviewPlayback || (PlayerStatus != EMovieScenePlayerStatus::Jumping && PlayerStatus != EMovieScenePlayerStatus::Stopped);
					AnimInstance->GetPlayer()->SetIsPreviewPlayer(bPreviewPlayback);
					AnimInstance->GetPlayer()->SetSequencePlaybackTime(AnimParams.SequencePtr.Get(), AnimParams.EvalTime, bFireNotifies, AnimInstance, bLooping);
				}
			}
		}
	}
};

FPaperZDMovieSceneAnimationTemplate::FPaperZDMovieSceneAnimationTemplate(const UPaperZDMovieSceneAnimationSection& InSection)
	: Params(InSection.Params, InSection.GetInclusiveStartFrame(), InSection.GetExclusiveEndFrame())
{
}

void FPaperZDMovieSceneAnimationTemplate::Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const
{
	// calculate the time at which to evaluate the animation
	float EvalTime = Params.MapTimeToAnimation(Context.GetTime(),Context.GetFrameRate(), Params.Animation);
	float Weight = Params.MapTimeToWeight(Context.GetTime(), Context.GetFrameRate());
	
	// Create the execution token with the minimal AnimParams
	FMinimalAnimParameters AnimParams(
	Params.Animation, EvalTime, GetTypeHash(PersistentData.GetSectionKey()),  Weight
	);
	ExecutionTokens.Add(FPaperZDAnimationExecutionToken(AnimParams));
}

void FPaperZDMovieSceneAnimationTemplate::Initialize(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) const
{
#if WITH_EDITOR
	//This allows the editor to recover previous version Animation sections, that only have the old Identifier
	if (Params.Animation == nullptr && !Params.AnimationName.IsNone())
	{
		TArray<UPaperZDAnimSequence*> AnimSequences;
		for (TWeakObjectPtr<> Object : Player.FindBoundObjects(Operand))
		{
			if (APaperZDCharacter* Character = Cast<APaperZDCharacter>(Object.Get()))
			{
				AnimSequences = Character->AnimationBlueprint->GetAnimSequences();
				break;
			}
		}

		//Search for the anim sequence
		UPaperZDAnimSequence* ValidSequence = nullptr;
		for (UPaperZDAnimSequence* Seq : AnimSequences)
		{
			if (Seq->Identifier.IsEqual(Params.AnimationName))
			{
				ValidSequence = Seq;
				break;
			}
		}
		
		if (ValidSequence && Params.Section.IsValid())
		{
			Params.Section->Params.Animation = ValidSequence;
			Params.Section->Params.AnimationName = NAME_None;
			Params.Section->MarkPackageDirty();
		}
	}
#endif
}
