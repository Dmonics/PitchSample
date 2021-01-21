// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "PaperZDCharacter.h"
#include "PaperZD.h"
#include "PaperZDAnimBP.h"
#include "PaperZDAnimInstance.h"
#include "PaperFlipbookComponent.h"
#include "AnimSequences/PaperZDAnimSequence_Flipbook.h"
#include "AnimSequences/Players/PaperZDAnimPlayer.h"

APaperZDCharacter::APaperZDCharacter(const FObjectInitializer& ObjectInitializer)
	: Super()
{
}

void APaperZDCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(AnimInstance)
		AnimInstance->Tick(DeltaSeconds);
}

void APaperZDCharacter::BeginPlay()
{
	Super::BeginPlay();

	//Force the creation of a fresh anim instance
	GetOrCreateAnimInstance(true);
}

void APaperZDCharacter::OnAnimationUpdated_Implementation(class UPaperFlipbook *From, class UPaperFlipbook* To)
{
}

UPaperZDAnimInstance* APaperZDCharacter::GetOrCreateAnimInstance(bool bForceCreation)
{
	//Clean the anim instance first if needed
	if (bForceCreation)
		AnimInstance = nullptr;

	//If no AnimBlueprint given, done
	if (!AnimInstanceClass)
		return nullptr;

	//@TODO: Patch until adding OnBlueprintCompiled listeners
	if (!(AnimInstance && AnimInstance->RootNode)) {
		AnimInstance = NewObject<UPaperZDAnimInstance>(this, AnimInstanceClass);
		AnimInstance->Init(this);
	}

	return AnimInstance;
}

UPaperZDAnimInstance* APaperZDCharacter::GetTransientAnimInstance()
{
	UPaperZDAnimInstance *TransientInstance = nullptr;
	if (AnimInstanceClass) {
		TransientInstance = NewObject<UPaperZDAnimInstance>(GetTransientPackage(), AnimInstanceClass);
		TransientInstance->Init(this);
	}

	return TransientInstance;
}

void APaperZDCharacter::PrepareForMovieSequence()
{
	if (AnimInstance)
	{
		AnimInstance->AnimationUpdateMode = EPaperZDAnimationMode::Custom;
	}

	OnPrepareForMovieSequence();
}

void APaperZDCharacter::RestoreFromMovieSequence()
{
	if (AnimInstance)
	{
		AnimInstance->AnimationUpdateMode = EPaperZDAnimationMode::StateMachine;
	}

	OnRestoreFromMovieSequence();
}

void APaperZDCharacter::ConfigurePlayer_Implementation(UPaperZDAnimPlayer* Player)
{
	//We need only configure the Flipbook component
	Player->RegisterRenderComponent(UPaperZDAnimSequence_Flipbook::StaticClass(), GetSprite());
}

#if WITH_EDITOR
void APaperZDCharacter::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetName().Equals(TEXT("AnimationBlueprint")))
	{
		if (AnimationBlueprint)
		{
			AnimInstanceClass = AnimationBlueprint->GeneratedClass;
		}
		else
		{
			AnimInstanceClass = nullptr;
		}

		if (AnimInstance)
		{
			GetOrCreateAnimInstance(true);
		}
	}
	else if (!PropertyChangedEvent.Property && AnimationBlueprint)
	{
		AnimInstanceClass = AnimationBlueprint->GeneratedClass;
	}
}

void APaperZDCharacter::OnAnimBPCompiled()
{
	//If this instance already has a AnimInstance running we need to invalidate it
	//forcing a recreation of the AnimInstance won't work on this stage, because PostCompile gets called before the TermDefaults are copied to the CDO
	//meaning the RootNode wouldn't been set, and hence, the Init method would fail.
	AnimInstance = nullptr;
}
#endif