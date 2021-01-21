// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "EditorViewportClient.h"
#include "SEditorViewport.h"
#include "SScrubControlPanel.h"
#include "PreviewScene.h"

class FPaperZDAnimSequenceViewportClient : public FEditorViewportClient
{
public:
	FPaperZDAnimSequenceViewportClient(const TAttribute<class UPaperZDAnimSequence*>& InAnimSequence);
	~FPaperZDAnimSequenceViewportClient();

	// FViewportClient interface
	virtual void Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	virtual void DrawCanvas(FViewport& InViewport, FSceneView& View, FCanvas& Canvas) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupViewForRendering(FSceneViewFamily& ViewFamily, FSceneView& View) override;
	// End of FViewportClient interface

	//Playback Related
	float GetFramesPerSecond() const;
	int32 GetCurrentFrame() const;
	void SetCurrentFrame(int32 NewIndex);
	float GetPlaybackPosition() const;
	uint32 GetTotalFrameCount() const;
	uint32 GetTotalFrameCountPlusOne() const;
	float GetTotalSequenceLength() const;
	void SetPlaybackPosition(float NewTime);
	bool IsLooping() const;
	EPlaybackMode::Type GetPlaybackMode() const; 
	float GetViewRangeMin() const;
	float GetViewRangeMax() const;
	void SetViewRange(float NewMin, float NewMax);

	FReply OnClick_Forward();
	FReply OnClick_Forward_Step();
	FReply OnClick_Forward_End();
	FReply OnClick_Backward();
	FReply OnClick_Backward_Step();
	FReply OnClick_Backward_End();
	FReply OnClick_ToggleLoop();
	//End of PlaybackRelated

private:
	void UpdateAudioListener(const FSceneView& View);

protected:
	// Range of times currently being viewed
	mutable float ViewInputMin;
	mutable float ViewInputMax;
	mutable float LastObservedSequenceLength;

private:
	// The preview scene
	FPreviewScene OwnedPreviewScene;

	//Flag to mark the zoom of sprite (on first tick)
	bool bSpriteZoomed;

	//If we are looping
	bool bLooping;

	// The AnimSequence to modify
	TAttribute<class UPaperZDAnimSequence*> AnimSequenceBeingEdited;

	// Render component for the sprite being edited
	TWeakObjectPtr<class UPrimitiveComponent> AnimatedRenderComponent;

	//The PaperZDPlayer that will manage the AnimSequence
	UPROPERTY()
	class UPaperZDAnimPlayer* Player;
};
