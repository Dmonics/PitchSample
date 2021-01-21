// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "AnimSequences/PaperZDAnimSequence.h"
#include "PaperZDAnimPlayer.generated.h"

UENUM(BlueprintType)
enum class EAnimPlayerPlaybackMode : uint8
{
	Forward UMETA(DisplayName="Forward"),
	Reversed UMETA(DisplayName = "Reversed")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPlaybackSequenceChangedSignature, const UPaperZDAnimSequence*, From, const UPaperZDAnimSequence*, To, float, CurrentProgress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlaybackSequenceCompleteSignature, const UPaperZDAnimSequence*, AnimSequence);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlaybackSequenceLoopedSignature, const UPaperZDAnimSequence*, AnimSequence);

UCLASS()
class PAPERZD_API UPaperZDAnimPlayer : public UObject
{
	GENERATED_BODY()

	UPaperZDAnimPlayer();

public:
	/**
	 * Delegate called when the player just starts playing a new sequence, different from the one played last frame.
	 * Called after the player issues a "Reset" on the playback time, but before actually updating the RenderPlayback on the RenderComponent.
	 * Gives a chance to modify any startup config needed for the newly placed AnimSequence
	 */
	UPROPERTY(BlueprintAssignable, Category = "Playback")
	FOnPlaybackSequenceChangedSignature OnPlaybackSequenceChanged;

	/**
	 * Called when the currently played AnimSequence reaches its end. Will only be called for non-looping sequences.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Playback")
	FOnPlaybackSequenceCompleteSignature OnPlaybackSequenceComplete;

	/**
	 * Called when the currently played AnimSequence loops. Only called if the playback is set to loop.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Playback")
	FOnPlaybackSequenceLoopedSignature OnPlaybackSequenceLooped;

	/* Obtain the last played time of the last played AnimSequence */
	UFUNCTION(BlueprintPure, Category = "Playback")
	float GetCurrentPlaybackTime() const;

	/* Obtain the progress, ranging from [0-1] of the current AnimSequence being played. */
	UFUNCTION(BlueprintPure, Category = "Playback")
	float GetPlaybackProgress() const;

	/* Sets up this player's current time */
	UFUNCTION(BlueprintCallable, Category = "Playback")
	void SetCurrentPlaybackTime(float InCurrentTime);

	/* Reset the playback of this player and sequence to its initial state. */
	UFUNCTION(BlueprintCallable, Category = "Playback")
	void Reset();

	/* Changes if this is a preview player or not. Some AnimSequences play or configure differently on PreviewMode. */
	void SetIsPreviewPlayer(bool bInPreviewPlayer);

	/* Ticks the playback of this player. */
	void TickPlayback(const UPaperZDAnimSequence* AnimSequence, const float DeltaTime, const bool bLooping, class UPaperZDAnimInstance *OwningInstance = nullptr);

	/**
	 * Manually sets the playback time, triggering any notify if enabled.
	 * @param AnimSequence		AnimSequence to update
	 * @param Time				Current time that needs to be played.
	 * @param bFireNotifies		If the sequence should trigger its notifies or ignore them.
	 * @param OwningInstance	AnimInstance responsible for calling this, can be null.
	 * @param bLooping			If this time update was called on a playback that will be automatically looped.
	 */
	void SetSequencePlaybackTime(const UPaperZDAnimSequence* AnimSequence, float Time, const bool bFireNotifies, class UPaperZDAnimInstance* OwningInstance = nullptr, bool bLooping = false);

	/**
	 * Registers a render component to be used with the given type of AnimSequence class.
	 * AnimSequences of that class will be given the registered render component to update playback when necessary.
	 * @param AnimSequenceClass			Class of AnimSequence to be registered to the given render component.
	 * @param RenderComponent			RenderComponent to be registered.
	 */
	UFUNCTION(BlueprintCallable, Category = "Playback")
	void RegisterRenderComponent(TSubclassOf<UPaperZDAnimSequence> AnimSequenceClass, UPrimitiveComponent* RenderComponent);

	UFUNCTION(BlueprintPure, Category = "Playback")
	bool IsPlaying() const { return bPlaying; }

	UFUNCTION(BlueprintCallable, Category = "Playback")
	void ResumePlayback();

	UFUNCTION(BlueprintCallable, Category = "Playback")
	void PausePlayback();

	/**
	 * Clears the playback time buffer variable, necessary when the player will change animations but needs to invalidate the current time from external 
	 * getters.
	 */
	void ClearPlaybackBuffer();

private:
	/* Called when a new sequence is played. */
	void SequencePlaybackChanged(const UPaperZDAnimSequence* AnimSequence, bool bLooping);

public:
	UPROPERTY(BlueprintReadWrite, Category = "Playback")
	EAnimPlayerPlaybackMode PlaybackMode;

private:
	UPROPERTY(transient)
	TMap<UClass*, class UPrimitiveComponent*> RenderComponentMap;

	//Holds the last played sequence, so it can reset after changing
	UPROPERTY(transient)
	TWeakObjectPtr<const UPaperZDAnimSequence> LastPlayedSequence;

	//State
	/* Last played time on the anim sequence. */
	float CurrentPlaybackTime;

	/* Buffered playback time that can manually be cleared when changing animations and the time hasn't been cleared automatically yet. */
	float BufferPlaybackTime;

	bool bPlaying;
	bool bNotifyCompletion;
	bool bPreviewPlayer;
};
