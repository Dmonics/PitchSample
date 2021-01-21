// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Evaluation/MovieSceneEvalTemplate.h"
#include "Sequencer/PaperZDMovieSceneAnimationSection.h"
#include "PaperZDMovieSceneAnimationTemplate.generated.h"

USTRUCT()
struct FPaperZDMovieSceneAnimationSectionTemplateParams : public FPaperZDMovieSceneAnimationParams
{
	GENERATED_BODY()

	FPaperZDMovieSceneAnimationSectionTemplateParams() {}

	FPaperZDMovieSceneAnimationSectionTemplateParams(const FPaperZDMovieSceneAnimationParams& BaseParams, FFrameNumber InSectionStartTime, FFrameNumber InSectionEndTime)
		: FPaperZDMovieSceneAnimationParams(BaseParams)
		, SectionStartTime(InSectionStartTime)
		, SectionEndTime(InSectionEndTime)
	{}

	float MapTimeToAnimation(FFrameTime InPosition, FFrameRate InFrameRate, class UPaperZDAnimSequence *Sequence) const;
	float MapTimeToWeight(FFrameTime InPosition, FFrameRate InFrameRate) const;

	UPROPERTY()
	FFrameNumber SectionStartTime;

	UPROPERTY()
	FFrameNumber SectionEndTime;
};

USTRUCT()
struct FPaperZDMovieSceneAnimationTemplate : public FMovieSceneEvalTemplate
{
	GENERATED_BODY()

	FPaperZDMovieSceneAnimationTemplate() {}
	
	FPaperZDMovieSceneAnimationTemplate(const UPaperZDMovieSceneAnimationSection& Section);

	virtual UScriptStruct& GetScriptStructImpl() const override { return *StaticStruct(); }

	virtual void SetupOverrides() override
	{
		EnableOverrides(RequiresInitializeFlag);
	}

	virtual void Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const override;
	virtual void Initialize(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) const override;
	
	UPROPERTY()
	FPaperZDMovieSceneAnimationSectionTemplateParams Params;
};
