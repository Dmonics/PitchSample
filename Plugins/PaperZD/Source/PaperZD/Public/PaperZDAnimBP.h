// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/Blueprint.h"
#include "PaperZDAnimBP.generated.h"

#if WITH_EDITOR
class UEdGraph;
class UPaperZDAnimBP;
class UPaperZDAnimNode;

/**
 * Proxy interface that handles operations called from the runtime section to the editor section.
 * Given that the runtime module cannot have dependence on the Editor Module, this proxy serves as the Editor-only connection
 */
class IPaperZDEditorProxy
{
public:
	virtual ~IPaperZDEditorProxy() {}

	// Called when creating a new sound AnimBP. 
	virtual UEdGraph* CreateNewAnimationGraph(UPaperZDAnimBP* InAnimBP) = 0;

	// Sets up an AnimNode 
	virtual void SetupAnimNode(UEdGraph* AnimGraph, UPaperZDAnimNode* AnimNode, bool bSelectNewNode) = 0;

	//Handles deep duplication policy of the Animation Blueprint
	virtual void HandleDuplicateAnimBP(UPaperZDAnimBP* InAnimBP) = 0;

	//Methods for updating version
	virtual void UpdateVersionToAnimSequences(UPaperZDAnimBP* InAnimBP) = 0;
	virtual void UpdateVersionToAnimNodeOuterFix(UPaperZDAnimBP* InAnimBP) = 0;
	virtual void UpdateVersionToAnimSequenceCategoryAdded(UPaperZDAnimBP* InAnimBP) = 0;
	virtual void UpdateVersionToAnimSequenceAsStandaloneAsset(UPaperZDAnimBP* InAnimBP) = 0;
};
#endif

class UEdGraph;
class UPaperZDAnimSequence;
/**
 * Class responsible of driving animation for 2d characters.
 * Compiles into PaperZDAnimInstance, which is the runtime compiled class that works ingame.
 */
UCLASS(Blueprintable)
class PAPERZD_API UPaperZDAnimBP : public UBlueprint
{
	GENERATED_UCLASS_BODY()


#if WITH_EDITORONLY_DATA
public:
	UPROPERTY()
	class UEdGraph* AnimationGraph;

	UPROPERTY()
	TArray<FName> RegisteredNotifyNames;

protected:
	/* @DEPRECATED: Internal array that holds the private AnimSequences (non-standalone) */
	UPROPERTY()
	TArray<class UPaperZDAnimSequence*> AnimSequences_DEPRECATED;
#endif

public:

	virtual void PostLoad() override;
	virtual void Serialize(FArchive& Ar) override;
#if WITH_EDITOR
	virtual void PostInitProperties() override;
	virtual void PostDuplicate(bool bDuplicateForPIE) override;

	/* Unregisters a custom notify from the function map. */
	void UnregisterCustomNotify(const FName& NotifyName);

	/**
	 * Unregisters a custom notify from the function map.
	 * @param NotifyName	The name of the notify to register
	 * @return				Whether the notify was registered or not (in case it already existed)
	 */
	bool RegisterCustomNotify(const FName& NotifyName);
#endif

	template <class T>
	T* ConstructAnimNode(TSubclassOf<class UPaperZDAnimNode> AnimNodeClass = T::StaticClass(), bool bSelectNewNode = true)
	{
		// Set flag to be transactional so it registers with undo system - Register this RuntimeNode with the outermost package, so it can be loaded on a cooking build
		T* AnimNode = NewObject<T>(this, AnimNodeClass, NAME_None, RF_Transactional);
#if WITH_EDITOR
		SetupAnimNode(AnimNode, bSelectNewNode);
#endif
		return AnimNode;
	}

public:	
#if WITH_EDITOR

	// Set up EdGraph parts of a SoundNode 
	void SetupAnimNode(class UPaperZDAnimNode *InAnimNode, bool bSelectNewNode);
	
	//Create the basic sound graph
	void CreateGraph();

	// Get the EdGraph of Animation
	class UEdGraph* GetGraph();

	// Sets the Animation Editor Proxy
	static void SetAnimationEditor(TSharedPtr<IPaperZDEditorProxy> InAnimationEditorProxy);

	// Gets the sound cue graph editor implementation.
	static TSharedPtr<IPaperZDEditorProxy> GetEditorProxy();

	/* Searches through the AssetRegistry modules for AnimSequences that are registered to this AnimBP*/
	TArray<UPaperZDAnimSequence*> GetAnimSequences() const;	

	/* Called just before compiling this AnimBP, gives the chance to order some functions for the compiler. */
	void OnPreCompile();

	/* Called just after compiling this AnimBP, gives the chance to order some functions for the compiler. */
	void OnPostCompile();

	/* @DEPRECATED: Creates an internal (non-standalone) AnimSequence. Maintained for version support */
	UPaperZDAnimSequence* CreateAnimSequence();

private:
	/* @DEPRECATED: Creates a unique AnimSequence name, using the internal array of AnimSequences and incrementing an int at the end. Maintained for version support */
	FName CreateUniqueSequenceDisplayName() const;

protected:
	// Ptr to interface to animation editor operations. 
	static TSharedPtr<IPaperZDEditorProxy> EditorProxy;

	//Template to use when naming a new AnimSequence
	static const FString SequenceNameTemplate;

	//Friendship for access to version updating
	friend class FPaperZDRuntimeEditorProxy;
#endif
};
