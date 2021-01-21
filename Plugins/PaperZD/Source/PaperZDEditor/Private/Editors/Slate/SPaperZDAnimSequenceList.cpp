// Copyright 2017-2018 CriticalFailure Studio / Carlos Ibanez Ch.

#include "SPaperZDAnimSequenceList.h"
#include "AnimSequences/PaperZDAnimSequence.h"
#include "Editors/Util/PaperZDEditorStyle.h"
#include "EditorStyleSet.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Misc/FileHelper.h"

#define LOCTEXT_NAMESPACE "PaperZDAnimSequenceList"

namespace SequenceAssetUtils
{
	/* Handles some of the rules for validating AssetNaming, copied from ContentBrowserUtils, which is right now private and can't be used directly */
	bool IsValidObjectPathForCreate(const FString& ObjectPath, FText& OutErrorMessage, bool bAllowExistingAsset = false)
	{
		const FString ObjectName = FPackageName::ObjectPathToObjectName(ObjectPath);

		// Make sure the name is not already a class or otherwise invalid for saving
		if (!FFileHelper::IsFilenameValidForSaving(ObjectName, OutErrorMessage))
		{
			// Return false to indicate that the user should enter a new name
			return false;
		}

		// Make sure the new name only contains valid characters
		if (!FName::IsValidXName(ObjectName, INVALID_OBJECTNAME_CHARACTERS INVALID_LONGPACKAGE_CHARACTERS, &OutErrorMessage))
		{
			// Return false to indicate that the user should enter a new name
			return false;
		}

		// Make sure we are not creating an FName that is too large
		if (ObjectPath.Len() > NAME_SIZE)
		{
			// This asset already exists at this location, inform the user and continue
			OutErrorMessage = LOCTEXT("AssetNameTooLong", "This asset name is too long. Please choose a shorter name.");
			// Return false to indicate that the user should enter a new name
			return false;
		}

		// Check for an existing asset, unless it we were asked not to.
		if (!bAllowExistingAsset)
		{
			FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
			FAssetData ExistingAsset = AssetRegistryModule.Get().GetAssetByObjectPath(FName(*ObjectPath));
			if (ExistingAsset.IsValid())
			{
				// This asset already exists at this location, inform the user and continue
				OutErrorMessage = FText::Format(LOCTEXT("RenameAssetAlreadyExists", "An asset already exists at this location with the name '{0}'."), FText::FromString(ObjectName));

				// Return false to indicate that the user should enter a new name
				return false;
			}
		}

		return true;
	}
}

template<typename ItemType>
class SSequenceCategoryNodeTableRow : public STableRow < ItemType >
{
public:
	SLATE_BEGIN_ARGS(SSequenceCategoryNodeTableRow)
	{}
	SLATE_DEFAULT_SLOT(typename SSequenceCategoryNodeTableRow::FArguments, Content)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
	{
		STableRow<ItemType>::ChildSlot
			.Padding(0.0f, 2.0f, 0.0f, 0.0f)
			[
				SAssignNew(ContentBorder, SBorder)
				.BorderImage(this, &SSequenceCategoryNodeTableRow::GetBackgroundImage)
			.Padding(FMargin(0.0f, 3.0f))
			.BorderBackgroundColor(FLinearColor(.6, .6, .6, 1.0f))
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(2.0f, 2.0f, 2.0f, 2.0f)
			.AutoWidth()
			[
				SNew(SExpanderArrow, STableRow< ItemType >::SharedThis(this))
			]

		+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			[
				InArgs._Content.Widget
			]
			]
			];

		STableRow < ItemType >::ConstructInternal(
			typename STableRow< ItemType >::FArguments()
			.Style(FEditorStyle::Get(), "DetailsView.TreeView.TableRow")
			.ShowSelection(false),
			InOwnerTableView
		);
	}

	const FSlateBrush* GetBackgroundImage() const
	{
		if (STableRow<ItemType>::IsHovered())
		{
			return STableRow<ItemType>::IsItemExpanded() ? FEditorStyle::GetBrush("DetailsView.CategoryTop_Hovered") : FEditorStyle::GetBrush("DetailsView.CollapsedCategory_Hovered");
		}
		else
		{
			return STableRow<ItemType>::IsItemExpanded() ? FEditorStyle::GetBrush("DetailsView.CategoryTop") : FEditorStyle::GetBrush("DetailsView.CollapsedCategory");
		}
	}

	virtual void SetContent(TSharedRef< SWidget > InContent) override
	{
		ContentBorder->SetContent(InContent);
	}

	virtual void SetRowContent(TSharedRef< SWidget > InContent) override
	{
		ContentBorder->SetContent(InContent);
	}

private:
	TSharedPtr<SBorder> ContentBorder;
};

//AnimSequenceNode
FText FAnimSequenceNode::GetNodeNameAsText() const
{
	if (Type == EAnimSequenceNodeType::Category)
	{
		return FText::FromName(CategoryName);
	}

	return SequencePtr.IsValid() ? FText::FromName(SequencePtr->GetSequenceName()) : FText::FromString(TEXT("INVALID"));
}

void FAnimSequenceNode::AddChildren(TSharedPtr<FAnimSequenceNode> Node)
{
	if (Type == EAnimSequenceNodeType::Category)
	{
		Node->CategoryName = this->CategoryName;
		Children.Add(Node);
	}
}

//AnimSequenceList
void SPaperZDAnimSequenceList::Construct(const FArguments &InArgs)
{
	OnGetAnimSequences = InArgs._OnGetAnimSequences;
	OnAnimSequenceSelected = InArgs._OnAnimSequenceSelected;
	AllowsRenaming = InArgs._AllowsRenaming;
	
	TreeView = SNew(STreeView<TSharedPtr<FAnimSequenceNode>>)
		.ItemHeight(24.0f)
		.TreeItemsSource(&TreeDataSource)
		.OnSelectionChanged(this, &SPaperZDAnimSequenceList::OnNodeSelected)
		.OnGenerateRow(TSlateDelegates<TSharedPtr<FAnimSequenceNode>>::FOnGenerateRow::CreateSP(this, &SPaperZDAnimSequenceList::GenerateRow))
		.OnGetChildren(this, &SPaperZDAnimSequenceList::OnGetChildrenForCategory)
		.OnExpansionChanged(this, &SPaperZDAnimSequenceList::OnExpansionChanged)
		.OnContextMenuOpening(InArgs._OnContextMenuOpening);
	
	RequestRefresh();

	//Paste all
	ChildSlot
	[
		TreeView.ToSharedRef()
	];
}

void SPaperZDAnimSequenceList::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	//Check the validity of the categories
	for (TSharedPtr<FAnimSequenceNode> Node : TreeDataSource)
	{
		for (TSharedPtr<FAnimSequenceNode> ChildNode : Node->Children)
		{
			if (ChildNode->SequencePtr.IsValid() && ChildNode->CategoryName != ChildNode->SequencePtr->Category)
			{
				RequestRefresh();
				return;
			}
		}
	}
}

void SPaperZDAnimSequenceList::RefreshListSource()
{	
	//Clear the Edit Mapping
	SequenceTextBlockMap.Empty();

	//@TODO: pending filtering
	TArray<UPaperZDAnimSequence*> Sequences = OnGetAnimSequences.Execute();

	//First we should organize the sequences
	Sequences.Sort([](const UPaperZDAnimSequence &Left, const UPaperZDAnimSequence &Right) {
		//If same category, then we order by name
		if (Left.Category == Right.Category)
		{
			return Left.GetSequenceName().LexicalLess(Right.GetSequenceName());
		}
			

		return Left.Category.LexicalLess(Right.Category);
	});

	TreeDataSource.Empty();
	TMap<FName, TSharedPtr<FAnimSequenceNode>> NodeSearchMap;
	for (UPaperZDAnimSequence *Sequence : Sequences)
	{
		//We only create the category node once per category
		if (!NodeSearchMap.Contains(Sequence->Category))
		{
			TSharedPtr<FAnimSequenceNode> CategoryNode = MakeShareable(new FAnimSequenceNode(Sequence->Category));
			NodeSearchMap.Add(Sequence->Category, CategoryNode);
			TreeDataSource.Add(CategoryNode);
		}

		TSharedPtr<FAnimSequenceNode> CategoryNode = NodeSearchMap[Sequence->Category];
		CategoryNode->AddChildren(MakeShareable(new FAnimSequenceNode(Sequence)));
	}
}

void SPaperZDAnimSequenceList::RequestRefresh()
{
	RefreshListSource();
	TreeView->RequestTreeRefresh();

	for (TSharedPtr<FAnimSequenceNode> Node : TreeDataSource)
	{
		TreeView->SetItemExpansion(Node, Node->bIsExpanded);
	}
}

void SPaperZDAnimSequenceList::OnGetChildrenForCategory(TSharedPtr<FAnimSequenceNode> InItem, TArray< TSharedPtr<FAnimSequenceNode> >& OutChildren)
{
	if (InItem->Children.Num())
	{
		OutChildren = InItem->Children;
	}
}

void SPaperZDAnimSequenceList::OnNodeSelected(TSharedPtr<FAnimSequenceNode> InSelectedItem, ESelectInfo::Type SelectInfo)
{
	if (InSelectedItem.IsValid() && InSelectedItem->Type == EAnimSequenceNodeType::Leaf && InSelectedItem->SequencePtr.IsValid())
	{
		OnAnimSequenceSelected.ExecuteIfBound(InSelectedItem->SequencePtr.Get(), SelectInfo);
	}
}

void SPaperZDAnimSequenceList::OnExpansionChanged(TSharedPtr<FAnimSequenceNode> InSelectedItem, bool bIsExpanded)
{
	if (InSelectedItem.IsValid())
	{
		InSelectedItem->bIsExpanded = bIsExpanded;
	}
}

TSharedRef<ITableRow> SPaperZDAnimSequenceList::GenerateRow(TSharedPtr<FAnimSequenceNode> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	if (InItem->Type == EAnimSequenceNodeType::Category)
	{
		return SNew(SSequenceCategoryNodeTableRow< TSharedPtr<FAnimSequenceNode> >, OwnerTable)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(5.0f, 0.0f)
			.VAlign(EVerticalAlignment::VAlign_Center)
			[
				SNew(STextBlock).Text(TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SPaperZDAnimSequenceList::GetNodeNameAsText, InItem)))
			]
			
		];
	}
	else
	{
		TSharedPtr<SInlineEditableTextBlock> EditableTextBlock = SNew(SInlineEditableTextBlock)
			.Text(TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SPaperZDAnimSequenceList::GetNodeNameAsText, InItem)))
			.IsReadOnly(TAttribute<bool>::Create(TAttribute<bool>::FGetter::CreateSP(this, &SPaperZDAnimSequenceList::IsNameReadOnly)))
			.OnVerifyTextChanged(this, &SPaperZDAnimSequenceList::HandleVerifySequenceNameChanged, InItem)
			.OnTextCommitted(this, &SPaperZDAnimSequenceList::HandleSequenceNameCommited, InItem)
			.OnBeginTextEdit(this, &SPaperZDAnimSequenceList::HandleBeginRenameSequence, InItem);

		//Make sure to map this text block 
		SequenceTextBlockMap.Add(InItem->SequencePtr.Get(), EditableTextBlock);

		return SNew(STableRow<TSharedPtr<FAnimSequenceNode>>, OwnerTable)
			.Padding(2.0f)
			[
				SNew(SHorizontalBox)
				/*+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(SBorder)
					.Padding(4.0f)
					.BorderImage(FEditorStyle::GetBrush("PropertyEditor.AssetThumbnailShadow"))
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						[
							SNew(SImage)
							.Image(FPaperZDEditorStyle::Get().GetBrush("PaperZDEditor.Sequences.Avatar"))
						]
					]
				]*/
			+SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(5.0f, 0.0f)
			.VAlign(EVerticalAlignment::VAlign_Center)
			[
				EditableTextBlock.ToSharedRef()
			]

			];
	}
}

void SPaperZDAnimSequenceList::EnterRenameMode(UPaperZDAnimSequence* AnimSequence)
{
	if (AnimSequence)
	{
		//Try to find the corresponding bound editable text block
		TSharedPtr<SInlineEditableTextBlock>* InlineBlockPtr = SequenceTextBlockMap.Find(AnimSequence);
		if (InlineBlockPtr)
		{
			(*InlineBlockPtr)->EnterEditingMode();
		}
	}
}

void SPaperZDAnimSequenceList::ExitRenameMode()
{
	if (RenamingSequence.IsValid())
	{
		TSharedPtr<SInlineEditableTextBlock>* InlineBlockPtr = SequenceTextBlockMap.Find(RenamingSequence.Get());
		if (InlineBlockPtr)
		{
			(*InlineBlockPtr)->ExitEditingMode();
		}
	}

	RenamingSequence = nullptr;
}

void SPaperZDAnimSequenceList::HandleBeginRenameSequence(const FText& OriginalText, TSharedPtr<FAnimSequenceNode> InAnimNode)
{
	//Finish any attempt of renaming other sequence
	ExitRenameMode();

	//Store the currently renaming sequence
	RenamingSequence = InAnimNode->SequencePtr;
}

bool SPaperZDAnimSequenceList::IsNameReadOnly() const
{
	return !AllowsRenaming.Get();
}

bool SPaperZDAnimSequenceList::HandleVerifySequenceNameChanged(const FText& NewText, FText& OutErrorMessage, TSharedPtr<FAnimSequenceNode> InAnimNode)
{
	if (InAnimNode->SequencePtr.IsValid())
	{
		const FString NewNameString = NewText.ToString();
		const UPaperZDAnimSequence* EditingSequence = InAnimNode->SequencePtr.Get();
		const FAssetData SequenceAsset = FAssetData(EditingSequence);
		const FString NewObjectPath = SequenceAsset.PackagePath.ToString() / NewNameString + TEXT(".") + NewNameString;
		
		if (SequenceAsset.AssetName.ToString() != NewNameString)
		{
			//We use a namespace that works very similar to the ContentBrowserUtils but handles less cases for the moment
			return SequenceAssetUtils::IsValidObjectPathForCreate(NewObjectPath, OutErrorMessage);
		}
		else
		{
			//Uses the same name as the original, all good
			return true;
		}
	}
	else
	{
		//Should NEVER come here
		OutErrorMessage = LOCTEXT("RenameInvalidSequence", "Sequence is invalid");
		return false;
	}
}

void SPaperZDAnimSequenceList::HandleSequenceNameCommited(const FText& NewText, ETextCommit::Type CommitInfo, TSharedPtr<FAnimSequenceNode> InAnimNode)
{
	if (InAnimNode->SequencePtr.IsValid())
	{
		const FString NewNameString = NewText.ToString();
		UPaperZDAnimSequence* EditingSequence = InAnimNode->SequencePtr.Get();
		const FAssetData SequenceAsset = FAssetData(EditingSequence);

		//Only do a rename if the name really is different from the original
		if (SequenceAsset.AssetName.ToString() != NewNameString)
		{
			FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
			TArray<FAssetRenameData> AssetsAndNames;
			TWeakObjectPtr<UObject> EditingSequencePtr = TWeakObjectPtr<UObject>(EditingSequence);
			new(AssetsAndNames) FAssetRenameData(EditingSequencePtr, SequenceAsset.PackagePath.ToString(), NewNameString);
			AssetToolsModule.Get().RenameAssetsWithDialog(AssetsAndNames);
		}

	}

	//Don't overstay on this mode
	ExitRenameMode();
}

FText SPaperZDAnimSequenceList::GetNodeNameAsText(TSharedPtr<FAnimSequenceNode> InItem)
{
	//Just for safety
	return InItem->GetNodeNameAsText();
}

TArray<UPaperZDAnimSequence*> SPaperZDAnimSequenceList::GetSelectedItems() const
{
	TArray<UPaperZDAnimSequence*> SelectedSequences;

	for (TSharedPtr<FAnimSequenceNode> Node : TreeView->GetSelectedItems())
	{
		if (Node.IsValid() && Node->Type == EAnimSequenceNodeType::Leaf && Node->SequencePtr.IsValid())
		{
			SelectedSequences.Add(Node->SequencePtr.Get());
		}
	}

	return SelectedSequences;
}

int32 SPaperZDAnimSequenceList::GetNumItemsSelected() const
{
	//return ListView->GetNumItemsSelected();
	return TreeView->GetNumItemsSelected();
}

void SPaperZDAnimSequenceList::SetSelection(UPaperZDAnimSequence* Sequence)
{
	if (Sequence)
	{
		//Create a AnimNode and give it to the tree source
		FAnimSequenceNode* SequenceNode = new FAnimSequenceNode(Sequence);
		TreeView->SetSelection(MakeShareable(SequenceNode));
	}
	else
	{
		TreeView->ClearSelection();
	}
}

#undef LOCTEXT_NAMESPACE
