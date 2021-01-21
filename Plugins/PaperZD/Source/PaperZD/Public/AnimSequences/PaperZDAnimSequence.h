// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Containers/TransArray.h"
#include "Notifies/PaperZDAnimNotify_Base.h"
#include "PaperZDAnimSequence.generated.h"

DECLARE_DELEGATE(FOnPostEditUndo)

class UPaperFlipbook;

/**
 * Class used for editor display that holds the AnimNotifies in an orderly manner.
 * Doesn't get called when searching for notify triggering.
 */
UCLASS()
class PAPERZD_API UPaperZDAnimTrack : public UObject
{
	GENERATED_UCLASS_BODY()
		
public:
	UPaperZDAnimTrack(FVTableHelper& Helper)
		: Super(Helper),
		AnimNotifies(this)
	{
	}

	//@TODO: TTransArray doesn't support UPROPERTY, change for a better TransactionalArray
	//The transaction notifies
	TTransArray<class UPaperZDAnimNotify_Base *>AnimNotifies;

	//The color set
	UPROPERTY()
		FLinearColor Color = FLinearColor::Gray;

#if WITH_EDITOR
public:
	void PostEditUndo() override
	{
		CachedNotifies = AnimNotifies;
	}
#endif

private:
	UPROPERTY()
	TArray<class UPaperZDAnimNotify_Base *>CachedNotifies; //Array to avoid GC killing the notifies
};

/**
 * The AnimSequence is the class responsible of handling how a given Animation source plays on the registered RenderComponent 
 * and handling meta info like AnimNotifies.
 */
UCLASS(abstract, BlueprintType)
class PAPERZD_API UPaperZDAnimSequence : public UObject
{
	GENERATED_UCLASS_BODY()


#if WITH_EDITOR
		//Added for version update to AnimSequence only
		friend class FPaperZDRuntimeEditorProxy;
#endif

private:
	//Pointer to the AnimBP Asset this Animation Sequence belongs to (so its searchable on the Registry)
	UPROPERTY(AssetRegistrySearchable, Category = Animation, VisibleAnywhere)
	class UPaperZDAnimBP* AnimBP;

public:
	//Need this constructor because TTransArray doesn't have a DefaultConstructor, hot reloading assumes that
	UPaperZDAnimSequence(FVTableHelper& Helper)
		: Super(Helper)
		, AnimTracks(this)
	{
	}

	UPROPERTY()
	FName DisplayName_DEPRECATED; //@Deprecated

	UPROPERTY()
	UPaperFlipbook *PaperFlipbook_DEPRECATED; //@Deprecated
	
	/* This has been deprecated, but still serialized due to the nature of sequencer backwards support. Will remove on a later version */
	UPROPERTY()
	FName Identifier;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimSequence")
	FName Category;

	UPROPERTY()
	TArray<UPaperZDAnimNotify_Base*> AnimNotifies;

	//Delegate for PostEditChanges - Needed for the editor undo support
	FOnPostEditUndo OnPostEditUndo;

	//UPROPERTY() --> TTransArray doesn't support UPROPERTY
	TTransArray<UPaperZDAnimTrack *> AnimTracks;

	/* Default category name when creating an AnimSequence */
	static const FName DefaultCategory;

private:
	/* Cache of AnimTracks to avoid GC, as the TTransArray cannot make use of Unreal's reflection */
	UPROPERTY()
	TArray<UPaperZDAnimTrack *> CachedAnimTracks; 

	//Name of the AnimBP owner member name
	static const FName AnimBPMemberName;

public:
	/* Helper to get the AnimBP member name for the editor and others property windows */
	static FName GetAnimBPMemberName() { return UPaperZDAnimSequence::AnimBPMemberName; }

	/* Obtains the AnimBP Linked to this Sequence */
	UPaperZDAnimBP* GetAnimBP() const { return AnimBP; }

	/* Set the AnimBP Linked to this sequence. */
	void SetAnimBP(class UPaperZDAnimBP* InAnimBP);

	/* Get the AnimNotifies linked to this sequence. */
	TArray<UPaperZDAnimNotify_Base *> GetAnimNotifies() const;

	/* Originally meant to hold the Display name, can be overridden if you don't want the default FName to be used when referring to this AnimBP*/
	virtual FName GetSequenceName() const;

	/* Total duration of this Sequence, should be overridden */
	UFUNCTION(BlueprintPure, Category = "AnimSequence")
	virtual float GetTotalDuration() const;

	/* Frames por second, used to show the grid on the AnimSequence editor */
	UFUNCTION(BlueprintPure, Category = "AnimSequence")
	virtual float GetFramesPerSecond() const;

	/**
	 * Called when we first start playing this sequence on the AnimPlayer, should setup any needed initial values like changing Flipbooks/Animation name.
	 * @param RenderComponent		Component that will render this animation sequence.
	 * @param bLooping				If the sequence is being played on automatic looping mode.
	 * @param bIsPreviewPlayback	If this configured component will be used on an editor preview player.
	 */
	virtual void BeginSequencePlayback(class UPrimitiveComponent* RenderComponent, bool bLooping, bool bIsPreviewPlayback = false) const {}

	/**
	 * Method called each time the playback of the sequence changes, responsible of updating the render component.
	 * @param RenderComponent		Component registered to this sequence type which needs to be updated.
	 * @param Time					Current animation time to set.
	 * @param bIsPreviewPlayback	If this configured component will be used on an editor preview player.
	 */
	virtual void UpdateRenderPlayback(class UPrimitiveComponent* RenderComponent, const float Time, bool bIsPreviewPlayback = false) const {}

	/**
	 * Chance to configure global settings for the render component. Gets called on the CDO, so no instance values should be used.
	 * @param RenderComponent		Component to be configured.
	 * @param bIsPreviewPlayback	If this configured component will be used on an editor preview player.
	 */
	virtual void ConfigureRenderComponent(class UPrimitiveComponent* RenderComponent, bool bIsPreviewPlayback = false) const {}

	/* Returns the render component class to use on this sequence. Used for creating preview render components */
	virtual TSubclassOf<UPrimitiveComponent> GetRenderComponentClass() const;

#if WITH_EDITOR
	void PostEditUndo() override;
	void PostLoad() override;
	void PostDuplicate(bool bDuplicateForPIE) override;
	void PostRename(UObject* OldOuter, const FName OldName) override;

	//Notify Related
	int32 CreateTrack(int32 InsertInto = INDEX_NONE);
	void RemoveTrack(int32 Index);
	void AddNotifyToTrack(class UPaperZDAnimNotify_Base* Notify, int32 TrackIndex);
	void AddNotifyToTrack(TSubclassOf<class UPaperZDAnimNotify_Base> NotifyClass, int32 TrackIndex, FName NotifyName, float InitTime = 0.0f);
	void RemoveNotify(class UPaperZDAnimNotify_Base* Notify);
	void MoveNotify(class UPaperZDAnimNotify_Base* Notify, int32 ToTrack);
	const TArray<UPaperZDAnimTrack*> GetTracks() { return AnimTracks; }
	UPaperZDAnimTrack* GetTrack(int32 i) { return AnimTracks[i]; }
	void InitTracks();
	void CleanInvalidNodes(TArray<FName> ValidRegisteredNames);

private:
	int GetNotifyTrack(UPaperZDAnimNotify_Base* Notify);
	void UpdateCachedInfo();

	//Friendship for version updating
	friend class FPaperZDRuntimeEditorProxy;
#endif
};
