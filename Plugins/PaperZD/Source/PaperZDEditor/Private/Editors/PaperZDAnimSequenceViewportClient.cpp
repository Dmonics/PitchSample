// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "PaperZDAnimSequenceViewportClient.h"
#include "PaperZDEditor.h"
#include "AnimSequences/Players/PaperZDAnimPlayer.h"
#include "AnimSequences/PaperZDAnimSequence.h"
#include "AssetEditorModeManager.h"
#include "Utils.h"
#include "Engine/CollisionProfile.h"
#include "AudioDevice.h"
#include "GameFramework/WorldSettings.h"

FVector PaperAxisX(1.0f, 0.0f, 0.0f);
FVector PaperAxisY(0.0f, 0.0f, 1.0f);
FVector PaperAxisZ(0.0f, 1.0f, 0.0f);

FPaperZDAnimSequenceViewportClient::FPaperZDAnimSequenceViewportClient(const TAttribute<UPaperZDAnimSequence*>& InAnimSequence) : FEditorViewportClient(new FAssetEditorModeManager())
{
	AnimSequenceBeingEdited = InAnimSequence;

	//Config
	SetRealtime(true);
	DrawHelper.bDrawGrid = false;
	EngineShowFlags.DisableAdvancedFeatures();
	EngineShowFlags.SetCompositeEditorPrimitives(false);
	bLooping = true;
		
	// Create a render component for the sprite being edited
	AnimatedRenderComponent = NewObject<UPrimitiveComponent>(GetTransientPackage(), AnimSequenceBeingEdited.Get()->GetRenderComponentClass());
	AnimatedRenderComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	AnimatedRenderComponent->UpdateBounds();

	bSpriteZoomed = false;
	
	//Create the player and configure it
	Player = NewObject<UPaperZDAnimPlayer>();
	Player->SetIsPreviewPlayer(true);
	Player->RegisterRenderComponent(AnimSequenceBeingEdited.Get()->GetClass(), AnimatedRenderComponent.Get());
	
	//Add the player to the root set, when compiling the AnimBP the transient package gets collected, this makes sure the player only gets gc'ed when this window gets destroyed
	Player->AddToRoot();
	PreviewScene = &OwnedPreviewScene;
	PreviewScene->AddComponent(AnimatedRenderComponent.Get(), FTransform::Identity);

	// Set audio mute option
	UWorld* World = PreviewScene->GetWorld();
	if (World)
	{
		World->bAllowAudioPlayback = true;//!ConfigOption->bMuteAudio; @TODO: Add a new button for mutting

		if (FAudioDevice* AudioDevice = World->GetAudioDevice())
		{
			AudioDevice->SetUseAttenuationForNonGameWorlds(false);
		}
	}

	// We need to configure WorldSettings if we want to be able to see OneShot particles that aren't attached, as they get attached to this actor
	AWorldSettings* WorldSettings = GetWorld()->GetWorldSettings(true);
	WorldSettings->bEnableWorldBoundsChecks = false;
	WorldSettings->SetIsTemporarilyHiddenInEditor(false);
	
	// Get the correct general direction of the perspective mode; the distance doesn't matter much as we've queued up a deferred zoom that will calculate a much better distance
	SetViewModes(VMI_Lit, VMI_Lit);
	SetViewportType(LVT_OrthoXZ);
	SetInitialViewTransform(LVT_Perspective, -100.0f * PaperAxisZ, PaperAxisZ.Rotation().GetInverse(), 0.0f);
}

FPaperZDAnimSequenceViewportClient::~FPaperZDAnimSequenceViewportClient()
{
	//Make sure the player gets gc'ed now that this view is no longer valid
	Player->RemoveFromRoot();
}

// FViewportClient interface
void FPaperZDAnimSequenceViewportClient::Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	FEditorViewportClient::Draw(View, PDI);

	if (AnimatedRenderComponent.IsValid())
	{
		FUnrealEdUtils::DrawWidget(View, PDI, AnimatedRenderComponent->GetComponentToWorld().ToMatrixWithScale(), 0, 0, EAxisList::Screen, EWidgetMovementMode::WMM_Translate);
	}
}

void FPaperZDAnimSequenceViewportClient::DrawCanvas(FViewport& InViewport, FSceneView& View, FCanvas& Canvas)
{
	FEditorViewportClient::DrawCanvas(InViewport, View, Canvas);
}

void FPaperZDAnimSequenceViewportClient::Tick(float DeltaSeconds)
{
	//Make the player manager tick
	Player->TickPlayback(AnimSequenceBeingEdited.Get(), DeltaSeconds, bLooping);

	FIntPoint Size = Viewport->GetSizeXY();
	if (!bSpriteZoomed && (Size.X > 0) && (Size.Y > 0))
	{
		//Calculate the Zoomed Out Bounds
		const FBox BoundsToFocus = AnimatedRenderComponent->Bounds.GetBox();
		FocusViewportOnBox(BoundsToFocus, true /*bDeferZoomToSpriteIsInstant*/);
		bSpriteZoomed = true;
	}

	FEditorViewportClient::Tick(DeltaSeconds);

	if (!GIntraFrameDebuggingGameThread)
	{
		OwnedPreviewScene.GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
	}
}

void FPaperZDAnimSequenceViewportClient::SetupViewForRendering(FSceneViewFamily& ViewFamily, FSceneView& View)
{
	FEditorViewportClient::SetupViewForRendering(ViewFamily, View);

	if (bHasAudioFocus)
	{
		UpdateAudioListener(View);
	}
}
// End of FViewportClient interface

void FPaperZDAnimSequenceViewportClient::UpdateAudioListener(const FSceneView& View)
{
	UWorld* ViewportWorld = GetWorld();

	if (ViewportWorld)
	{
		if (FAudioDevice* AudioDevice = ViewportWorld->GetAudioDevice())
		{
			const FVector& ViewLocation = GetViewLocation();
			const FRotator& ViewRotation = GetViewRotation();

			FTransform ListenerTransform(ViewRotation);
			ListenerTransform.SetLocation(ViewLocation);

			AudioDevice->SetListener(ViewportWorld, 0, ListenerTransform, 0.0f);
		}
	}
}

float FPaperZDAnimSequenceViewportClient::GetPlaybackPosition() const
{
	return Player->GetCurrentPlaybackTime();
}

uint32 FPaperZDAnimSequenceViewportClient::GetTotalFrameCount() const
{
	return (uint32)FMath::CeilToInt(GetFramesPerSecond() * AnimSequenceBeingEdited.Get()->GetTotalDuration());
}

uint32 FPaperZDAnimSequenceViewportClient::GetTotalFrameCountPlusOne() const
{
	return GetTotalFrameCount() + 1;
}

float FPaperZDAnimSequenceViewportClient::GetTotalSequenceLength() const
{
	return AnimSequenceBeingEdited.Get()->GetTotalDuration();
}

void FPaperZDAnimSequenceViewportClient::SetPlaybackPosition(float NewTime)
{
	NewTime = FMath::Clamp<float>(NewTime, 0.0f, GetTotalSequenceLength());

	Player->SetSequencePlaybackTime(AnimSequenceBeingEdited.Get(), NewTime, false);
}

float FPaperZDAnimSequenceViewportClient::GetFramesPerSecond() const
{
	return AnimSequenceBeingEdited.Get()->GetFramesPerSecond();
}

int32 FPaperZDAnimSequenceViewportClient::GetCurrentFrame() const
{
	const int32 TotalLengthInFrames = GetTotalFrameCount();

	if (TotalLengthInFrames == 0)
	{
		return INDEX_NONE;
	}
	else
	{
		return FMath::Clamp<int32>((int32)(GetPlaybackPosition() * GetFramesPerSecond()), 0, TotalLengthInFrames - 1);
	}
}

void FPaperZDAnimSequenceViewportClient::SetCurrentFrame(int32 NewIndex)
{
	const int32 TotalLengthInFrames = GetTotalFrameCount();
	if (TotalLengthInFrames > 0)
	{
		int32 ClampedIndex = FMath::Clamp<int32>(NewIndex, 0, TotalLengthInFrames - 1);
		SetPlaybackPosition(ClampedIndex / GetFramesPerSecond());
	}
	else
	{
		SetPlaybackPosition(0.0f);
	}
}

FReply FPaperZDAnimSequenceViewportClient::OnClick_Forward()
{
	const bool bIsReverse = Player->PlaybackMode == EAnimPlayerPlaybackMode::Reversed;
	const bool bIsPlaying = Player->IsPlaying();
		
	if (bIsReverse && bIsPlaying)
	{
		// Play forwards instead of backwards
		Player->PlaybackMode = EAnimPlayerPlaybackMode::Forward;
	}
	else if (bIsPlaying)
	{
		// Was already playing forwards, so pause
		Player->PausePlayback();
	}
	else
	{
		// Was paused, start playing
		Player->ResumePlayback();
	}
	
	return FReply::Handled();
}

FReply FPaperZDAnimSequenceViewportClient::OnClick_Forward_Step()
{
	SetCurrentFrame(GetCurrentFrame() + 1);
	return FReply::Handled();
}

FReply FPaperZDAnimSequenceViewportClient::OnClick_Forward_End()
{
	Player->PausePlayback();
	SetPlaybackPosition(AnimSequenceBeingEdited.Get()->GetTotalDuration());
	return FReply::Handled();
}

FReply FPaperZDAnimSequenceViewportClient::OnClick_Backward()
{
	const bool bIsReverse = Player->PlaybackMode == EAnimPlayerPlaybackMode::Reversed;
	const bool bIsPlaying = Player->IsPlaying();

	if (bIsReverse && bIsPlaying)
	{
		// Was already playing backwards, so pause
		Player->PausePlayback();
	}
	else if (bIsPlaying)
	{
		// Play backwards instead of forwards
		Player->PlaybackMode = EAnimPlayerPlaybackMode::Reversed;
	}
	else
	{
		// Was paused, start reversing
		Player->PlaybackMode = EAnimPlayerPlaybackMode::Reversed;
		Player->ResumePlayback();
	}

	return FReply::Handled();
}

FReply FPaperZDAnimSequenceViewportClient::OnClick_Backward_Step()
{
	Player->PausePlayback();
	SetCurrentFrame(GetCurrentFrame() - 1);
	return FReply::Handled();
}

FReply FPaperZDAnimSequenceViewportClient::OnClick_Backward_End()
{
	Player->PausePlayback();
	SetPlaybackPosition(0.0f);
	return FReply::Handled();
}

FReply FPaperZDAnimSequenceViewportClient::OnClick_ToggleLoop()
{
	bLooping = !bLooping;
	return FReply::Handled();
}

EPlaybackMode::Type FPaperZDAnimSequenceViewportClient::GetPlaybackMode() const
{
	if (Player->IsPlaying())
	{
		return Player->PlaybackMode == EAnimPlayerPlaybackMode::Reversed ? EPlaybackMode::PlayingReverse : EPlaybackMode::PlayingForward;
	}
	else
	{
		return EPlaybackMode::Stopped;
	}
}

bool FPaperZDAnimSequenceViewportClient::IsLooping() const
{
	return bLooping;
}

float FPaperZDAnimSequenceViewportClient::GetViewRangeMin() const
{
	return ViewInputMin;
}

float FPaperZDAnimSequenceViewportClient::GetViewRangeMax() const
{
	// See if the flipbook changed length, and if so reframe the scrub bar to include the full length
	//@TODO: This is a pretty odd place to put it, but there's no callback for a modified timeline at the moment, so...
	const float SequenceLength = GetTotalSequenceLength();
	if (SequenceLength != LastObservedSequenceLength)
	{
		LastObservedSequenceLength = SequenceLength;
		ViewInputMin = 0.0f;
		ViewInputMax = SequenceLength;
	}

	return ViewInputMax;
}

void FPaperZDAnimSequenceViewportClient::SetViewRange(float NewMin, float NewMax)
{
	ViewInputMin = FMath::Max<float>(NewMin, 0.0f);
	ViewInputMax = FMath::Min<float>(NewMax, GetTotalSequenceLength());
}
