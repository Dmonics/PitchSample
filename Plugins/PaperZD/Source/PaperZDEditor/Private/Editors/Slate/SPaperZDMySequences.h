// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Slate/SPaperZDAnimSequenceList.h"
#include "Widgets/Input/SSearchBox.h"

class FPaperZDAnimBPEditor;
class UPaperZDAnimSequence;
struct FAssetData;

/**
* Generates a list that contains all the Sequences the bound AnimBP has assigned to it
*/
class SPaperZDMySequences : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPaperZDMySequences)
	{}
	SLATE_END_ARGS()

	/* Constructs the widget*/
	void Construct(const FArguments &InArgs, TWeakPtr<FPaperZDAnimBPEditor> InEditor);

	/* Destructor, needed for unbinding delegates */
	~SPaperZDMySequences();

	/* Called to construct the AddNew widget, that holds the AnimSequence construction */
	TSharedRef<SWidget> CreateAddNewMenuWidget();

	/* Builds the internal dropdown menu from AddNew */
	void BuildAddNewMenu(FMenuBuilder& MenuBuilder);

	/* Called after filtering changes, so we can update the list*/
	void OnFilterTextChanged(const FText& InFilterText);

	/* Refreshes the sequence list, called after data changess*/
	void ResetSequenceList();

private:
	TArray<UPaperZDAnimSequence*> GetAnimSequences();
	void HandleAnimSequenceSelected(UPaperZDAnimSequence* Sequence, ESelectInfo::Type SelectionType);
	void HandleCreateAnimSequence();
	void HandleDuplicateSelectedAnimSequence();
	void HandleDeleteSelectedAnimSequence();
	void HandleRenameSelectedAnimSequence();
	void HandleAnimSequenceFocusRequest(UPaperZDAnimSequence* Sequence);

	TSharedPtr<SWidget> GetListContextMenu();

	//Callback for Asset registry
	void OnAssetRegistryChanged(const FAssetData& InAssetData);

private:
	TSharedPtr<SPaperZDAnimSequenceList> AnimSequenceList;
	TSharedPtr<SSearchBox> SearchBox;
	TWeakPtr<FPaperZDAnimBPEditor> AnimBPEditor;
	TArray<UPaperZDAnimSequence*> FilteredSequences;
};
