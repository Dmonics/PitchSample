// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STreeView.h"
#include "AnimSequences/PaperZDAnimSequence.h"

enum class EAnimSequenceNodeType : uint8
{
	Category,
	Leaf
};

struct FAnimSequenceNode : TSharedFromThis<FAnimSequenceNode>
{
	FAnimSequenceNode(FName InName) : Type(EAnimSequenceNodeType::Category), CategoryName(InName), bIsExpanded(true) {}
	FAnimSequenceNode(UPaperZDAnimSequence* AnimSequence) : Type(EAnimSequenceNodeType::Leaf), SequencePtr(AnimSequence), bIsExpanded(true) {}

	EAnimSequenceNodeType Type;
	FName CategoryName;
	TArray<TSharedPtr<FAnimSequenceNode>> Children;
	TWeakObjectPtr<class UPaperZDAnimSequence> SequencePtr;
	bool bIsExpanded;

	FText GetNodeNameAsText() const;
	void AddChildren(TSharedPtr<FAnimSequenceNode> Node);
};


DECLARE_DELEGATE_RetVal(TArray<UPaperZDAnimSequence*>, FOnGetAnimSequences)
DECLARE_DELEGATE_TwoParams(FOnAnimSequenceSelected, UPaperZDAnimSequence*, ESelectInfo::Type)

class UPaperZDAnimSequence;
class SInlineEditableTextBlock;
class SPaperZDAnimSequenceList : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPaperZDAnimSequenceList)
		:_AllowsRenaming(false)
	{}
	SLATE_ATTRIBUTE(bool, AllowsRenaming)
	SLATE_EVENT(FOnGetAnimSequences, OnGetAnimSequences)
	SLATE_EVENT(FOnAnimSequenceSelected, OnAnimSequenceSelected)
	SLATE_EVENT(FOnContextMenuOpening, OnContextMenuOpening)
	SLATE_END_ARGS()

	/* Creates this Slate Widget */
	void Construct(const FArguments& InArgs);

	/* Asks this object to refresh the list, optionally deferring the update to the next frame. */
	void RequestRefresh();
		
	/* Obtains the items selected (clicked) */
	TArray<UPaperZDAnimSequence*> GetSelectedItems() const;

	/* How many items have been selected */
	int32 GetNumItemsSelected() const;

	/* Change the selection of the given sequence, calls the corresponding delegate */
	void SetSelection(UPaperZDAnimSequence* Sequence);

	/* Calls the bound EditbleTextBlock for this AnimSequence and handles rules and commiting rename operation. */
	void EnterRenameMode(UPaperZDAnimSequence* AnimSequence);

	/* Called to cancel every renaming mode attempt on this list */
	void ExitRenameMode();

private:
	/* Updates the list source*/
	void RefreshListSource();

	/* Negates the AllowsRenaming Attribute for use on ReadOnly */
	bool IsNameReadOnly() const;

	/* Handles the verification of the AnimSequence renaming */
	bool HandleVerifySequenceNameChanged(const FText& NewText, FText& OutErrorMessage, TSharedPtr<FAnimSequenceNode> InAnimNode);

	/* Handled renaming of the AnimSequence */
	void HandleSequenceNameCommited(const FText& NewText, ETextCommit::Type CommitInfo, TSharedPtr<FAnimSequenceNode> InAnimNode);

	/* Called when we just start renaminig a sequence */
	void HandleBeginRenameSequence(const FText& OriginalText, TSharedPtr<FAnimSequenceNode> InAnimNode);

protected:
	void OnGetChildrenForCategory(TSharedPtr<FAnimSequenceNode> InItem, TArray< TSharedPtr<FAnimSequenceNode> >& OutChildren);
	TSharedRef<ITableRow> GenerateRow(TSharedPtr<FAnimSequenceNode> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	FText GetNodeNameAsText(TSharedPtr<FAnimSequenceNode> InItem);
	void OnNodeSelected(TSharedPtr<FAnimSequenceNode> InSelectedItem, ESelectInfo::Type SelectInfo);
	void OnExpansionChanged(TSharedPtr<FAnimSequenceNode> InSelectedItem, bool bIsExpanded);

	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	/* Stores a direct map of the AnimSequence with its corresponding InlineEditableTextBlock widget */
	TMap<UPaperZDAnimSequence*, TSharedPtr<SInlineEditableTextBlock>> SequenceTextBlockMap;

	/* AnimSequence that is currently being renamed */
	TWeakObjectPtr<UPaperZDAnimSequence> RenamingSequence;
	
	//Event delegates
	FOnGetAnimSequences OnGetAnimSequences;
	FOnAnimSequenceSelected OnAnimSequenceSelected;

	/* If hits sequence list allows renaming the AnimSequences */
	TAttribute<bool> AllowsRenaming;

	//TreeView related
	TSharedPtr<STreeView<TSharedPtr<FAnimSequenceNode>>> TreeView;
	TArray<TSharedPtr<FAnimSequenceNode>> TreeDataSource;

	//@TODO deprecate
	//TSharedPtr<SListView<UPaperZDAnimSequence*>> ListView;
	TArray<UPaperZDAnimSequence*> FilteredSequences;
};
