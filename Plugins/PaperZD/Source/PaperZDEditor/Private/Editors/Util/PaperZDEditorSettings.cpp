// Copyright Carlos Ibanez Ch. - 2018
#include "PaperZDEditorSettings.h"

UPaperZDEditorSettings::UPaperZDEditorSettings() : Super()
{
	SequencePlacementPolicy = EAnimSequencePlacementPolicy::SubFolder;
	SequencePlacementFolderName = TEXT("AnimSequences");
	DuplicationPolicy = EAnimBlueprintDuplicationPolicy::IncludeUsedNotifies;
	SequenceDuplicationFolderName = TEXT("AnimSequences_Copy");
}

bool UPaperZDEditorSettings::CanEditChange(const UProperty* InProperty) const
{
	const FName PropertyName = InProperty->GetFName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UPaperZDEditorSettings, SequencePlacementFolderName))
	{
		return SequencePlacementPolicy == EAnimSequencePlacementPolicy::SubFolder;
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(UPaperZDEditorSettings, SequenceDuplicationFolderName))
	{
		return DuplicationPolicy == EAnimBlueprintDuplicationPolicy::IncludeUsedNotifies;
	}

	return Super::CanEditChange(InProperty);
}
