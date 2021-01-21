// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperZDEditorSettings.generated.h"

UENUM()
enum class EAnimSequencePlacementPolicy : uint8
{
	AlwaysAsk UMETA(Tooltip = "Ask the user where to put the sequence"),
	SameFolder UMETA(Tooltip = "Put the newly created sequence on the same folder as its parent AnimBP"),
	SubFolder UMETA(Tooltip = "Put the newly created sequence on a subfolder with an specified name")
};

UENUM()
enum class EAnimBlueprintDuplicationPolicy : uint8
{
	DuplicateOnlyWrapper UMETA(Tooltip="Duplicates the GraphNode, EventGraph and NotifyFunctions without duplicating the AnimSequences, which will have to be re-set manually"),
	IncludeUsedNotifies UMETA(Tooltip = "Duplicates everything and creates a copy for every AnimSequence currently being used on the Animation Graph to a new folder")
};

/**
 * Contains the Editor Only settings for PaperZD
 */
UCLASS(config=Editor, defaultconfig)
class UPaperZDEditorSettings : public UObject
{
	GENERATED_BODY()

public:
	UPaperZDEditorSettings();
	
	UPROPERTY(EditAnywhere, config, Category = "Animation Blueprint", meta=(Tooltip = "When creating a AnimSequence via the AnimBP Editor, where to put the AnimSequence"))
	EAnimSequencePlacementPolicy SequencePlacementPolicy;

	UPROPERTY(EditAnywhere, config, Category = "Animation Blueprint")
	FName SequencePlacementFolderName;

	UPROPERTY(EditAnywhere, config, Category = "Animation Blueprint", meta = (DisplayName = "AnimBP Duplication policy"))
	EAnimBlueprintDuplicationPolicy DuplicationPolicy;

	UPROPERTY(EditAnywhere, config, Category = "Animation Blueprint")
	FName SequenceDuplicationFolderName;

public:
	virtual bool CanEditChange(const UProperty* InProperty) const;
};
