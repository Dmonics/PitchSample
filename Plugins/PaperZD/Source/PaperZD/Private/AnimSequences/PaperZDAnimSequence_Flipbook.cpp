// Copyright 2017-2018 Critical Failure Studio.

#include "AnimSequences/PaperZDAnimSequence_Flipbook.h"
#include "PaperFlipbookComponent.h"
#include "PaperFlipbook.h"
#include "PaperZDAnimInstance.h"
#include "PaperZDCharacter.h"

void UPaperZDAnimSequence_Flipbook::BeginSequencePlayback(class UPrimitiveComponent* RenderComponent, bool bLooping, bool bIsPreviewPlayback /* = false */) const
{
	UPaperFlipbookComponent* Sprite = Cast<UPaperFlipbookComponent>(RenderComponent);
	if (Sprite)
	{
		//Update the Flipbook if needed
		if (Sprite->GetFlipbook() != Flipbook)
		{
			UPaperFlipbook* From = Sprite->GetFlipbook();
			Sprite->SetFlipbook(Flipbook);
		}
	}
}

void UPaperZDAnimSequence_Flipbook::UpdateRenderPlayback(class UPrimitiveComponent* RenderComponent, const float Time, bool bIsPreviewPlayback /* = false */) const
{
	UPaperFlipbookComponent* Sprite = Cast<UPaperFlipbookComponent>(RenderComponent);
	if (Sprite)
	{
#if WITH_EDITOR
		//On editor, the render playback begins (when the tab is opened), but the user can change the flipbook, so we need to be able to respond to those changes
		//not needed on runtime, as changing flipbooks on the AnimSequence isn't supported
		if (Sprite->GetFlipbook() != Flipbook)
		{
			UPaperFlipbook* From = Sprite->GetFlipbook();
			Sprite->SetFlipbook(Flipbook);
		}
#endif

		//We manage the time manually
		Sprite->SetPlaybackPosition(Time, false);
	}
}

float UPaperZDAnimSequence_Flipbook::GetTotalDuration() const
{
	return Flipbook ? Flipbook->GetTotalDuration() : 0.0f;
}

TSubclassOf<UPrimitiveComponent> UPaperZDAnimSequence_Flipbook::GetRenderComponentClass() const
{
	return UPaperFlipbookComponent::StaticClass();
}

void UPaperZDAnimSequence_Flipbook::ConfigureRenderComponent(class UPrimitiveComponent* RenderComponent, bool bIsPreviewPlayback /* = false */) const
{
	//Cast it to a PaperFlipbookComponent
	UPaperFlipbookComponent* Sprite = Cast<UPaperFlipbookComponent>(RenderComponent);

	if (Sprite)
	{
		Sprite->Stop();
		Sprite->SetLooping(false);
	}
}

float UPaperZDAnimSequence_Flipbook::GetFramesPerSecond() const
{
	//Default value is 15 fps
	return Flipbook ? Flipbook->GetFramesPerSecond() : 15.0f; 
}
