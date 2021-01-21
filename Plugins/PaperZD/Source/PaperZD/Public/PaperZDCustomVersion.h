// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Misc/Guid.h"

// Custom serialization version for all packages containing Paper2D asset types
struct FPaperZDCustomVersion
{
	enum Type
	{
		// Before any version changes were made in the plugin
		BeforeCustomVersionWasAdded = 0,

		// Added AnimSequences, Sequencer Support and Animation Queue
		AnimSequencesAdded = 1,

		//Re ordered Outers and copied the AnimNodes
		AnimNodeOuterFix = 2,

		//Added category to the sequences
		AnimSequenceCategoryAdded = 3,

		//Added AnimSequences as standalone UAssets
		AnimSequenceAsStandaloneAsset = 4,

		// -----<new versions can be added above this line>-------------------------------------------------
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1
	};

	// The GUID for this custom version number
	const static FGuid GUID;

private:
	FPaperZDCustomVersion() {}
};
