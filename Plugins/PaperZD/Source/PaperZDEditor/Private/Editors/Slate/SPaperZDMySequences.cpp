// Copyright 2017-2018 CriticalFailure Studio & Carlos Ibanez

#include "SPaperZDMySequences.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SSearchBox.h"
#include "EditorStyleSet.h"
#include "PaperZDAnimBPEditor.h"
#include "PaperZDAnimBP.h"
#include "AnimSequences/PaperZDAnimSequence.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "SKismetInspector.h"
#include "Editors/Util/PaperZDEditorSettings.h"

//Commands
#include "Util/PaperZDAnimBPEditorCommands.h"

//For constructing the AnimSequence
#include "Factories/PaperZDAnimSequenceFactory.h"
#include "AssetToolsModule.h"
#include "AssetRegistryModule.h"

//Modifying AnimSequences
#include "ObjectTools.h"

#define LOCTEXT_NAMESPACE "PaperZD_SMySequences"

void SPaperZDMySequences::Construct(const FArguments &InArgs, TWeakPtr<FPaperZDAnimBPEditor> InEditor)
{
	AnimBPEditor = InEditor;

	//Init the sequence datasource
	FilteredSequences = AnimBPEditor.Pin()->AnimBPBeingEdited->GetAnimSequences();

	//Bind to asset registry for deletion of AnimSequences
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetRegistryModule.Get().OnAssetAdded().AddSP(this, &SPaperZDMySequences::OnAssetRegistryChanged);
	AssetRegistryModule.Get().OnAssetRemoved().AddSP(this, &SPaperZDMySequences::OnAssetRegistryChanged);

	//Bind to the focus request delegate on the editor
	AnimBPEditor.Pin()->OnSequenceFocusRequest.BindSP(this, &SPaperZDMySequences::HandleAnimSequenceFocusRequest);

	//Create the Command mapping
	TSharedRef<FUICommandList> ToolkitCommands = AnimBPEditor.Pin()->GetToolkitCommands();
	ToolkitCommands->MapAction(
		FPaperZDAnimBPEditorCommands::Get().CreateAnimSequence,
		FExecuteAction::CreateSP(this, &SPaperZDMySequences::HandleCreateAnimSequence),
		FCanExecuteAction());

	ToolkitCommands->MapAction(
		FPaperZDAnimBPEditorCommands::Get().DuplicateAnimSequence,
		FExecuteAction::CreateSP(this, &SPaperZDMySequences::HandleDuplicateSelectedAnimSequence),
		FCanExecuteAction());

	ToolkitCommands->MapAction(
		FPaperZDAnimBPEditorCommands::Get().DeleteAnimSequence,
		FExecuteAction::CreateSP(this, &SPaperZDMySequences::HandleDeleteSelectedAnimSequence),
		FCanExecuteAction());

	ToolkitCommands->MapAction(
		FPaperZDAnimBPEditorCommands::Get().RenameAnimSequence,
		FExecuteAction::CreateSP(this, &SPaperZDMySequences::HandleRenameSelectedAnimSequence),
		FCanExecuteAction());

	//Create the Combo Button
	TSharedPtr<SComboButton> NewButton = SNew(SComboButton)
		.ComboButtonStyle(FEditorStyle::Get(), "ToolbarComboButton")
		.ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
		.ForegroundColor(FLinearColor::White)
		.ToolTipText(LOCTEXT("AddNewSequence", "Adds a new AnimSequence to this AnimBP."))
		.OnGetMenuContent(this, &SPaperZDMySequences::CreateAddNewMenuWidget)
		.HasDownArrow(true)
		.ContentPadding(FMargin(1, 0, 2, 0))
		//.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("MyBlueprintAddNewCombo")))
		//.IsEnabled(this, &SMyBlueprint::IsEditingMode)
		.ButtonContent()
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(FMargin(0, 1))
		[
			SNew(SImage)
			.Image(FEditorStyle::GetBrush("Plus"))
		]

	+ SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.AutoWidth()
		.Padding(FMargin(2, 0, 2, 0))
		[
			SNew(STextBlock)
			.Text(LOCTEXT("AddNew", "Add New"))
		]
		];

	//Create the searchbox
	SAssignNew(SearchBox, SSearchBox)
		.OnTextChanged(this,&SPaperZDMySequences::OnFilterTextChanged);

	//Create a ListView
	AnimSequenceList = SNew(SPaperZDAnimSequenceList)
		.OnGetAnimSequences(FOnGetAnimSequences::CreateSP(this, &SPaperZDMySequences::GetAnimSequences))
		.OnAnimSequenceSelected(this, &SPaperZDMySequences::HandleAnimSequenceSelected)
		.OnContextMenuOpening(this, &SPaperZDMySequences::GetListContextMenu)
		.AllowsRenaming(true);

	ChildSlot
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
				.Padding(4.0f)
				.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
				//.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("MyBlueprintPanel")))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0,0,2,0)
						[
							NewButton.ToSharedRef()
						]
						
						+SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.Padding(2,0,0,0)
						[
							SearchBox.ToSharedRef()
						]
					]
				]
			]
			+SVerticalBox::Slot()
			.Padding(0.0f,2.0f,0.0f, 0.0f)
			.FillHeight(1.0f)
				[
					SNew(SBorder)
					.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
					[
						AnimSequenceList.ToSharedRef()
					]
				]
		];
}

SPaperZDMySequences::~SPaperZDMySequences()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetRegistryModule.Get().OnAssetAdded().RemoveAll(this);
	AssetRegistryModule.Get().OnAssetRemoved().RemoveAll(this);

	//Optionally unbind (AnimBPEditor could be already deleted if we closed the window, instead of changing modes)
	if (AnimBPEditor.IsValid())
	{
		AnimBPEditor.Pin()->OnSequenceFocusRequest.Unbind();
	}
}

void SPaperZDMySequences::OnFilterTextChanged(const FText& InFilterText)
{
	FilteredSequences = AnimBPEditor.Pin()->AnimBPBeingEdited->GetAnimSequences();
	if (InFilterText.ToString() != "")
	{
		FilteredSequences = FilteredSequences.FilterByPredicate([InFilterText](const UPaperZDAnimSequence* ToFilter) {
			return ToFilter->GetSequenceName().ToString().Contains(InFilterText.ToString());
		});
	}

	AnimSequenceList->RequestRefresh();
}

void SPaperZDMySequences::OnAssetRegistryChanged(const FAssetData& InAssetData)
{
	//We need to check if the asset class is a AnimSequence (class should be loaded)
	const UClass* AssetClass = InAssetData.GetClass();
	if (AssetClass && AssetClass->IsChildOf(UPaperZDAnimSequence::StaticClass()))
	{
		ResetSequenceList();
	}
}

void SPaperZDMySequences::ResetSequenceList()
{
	//Reset the search
	SearchBox->SetText(FText::FromString(""));

	//AnimBPEditor could no longer valid, this is a border case in which we delete a related AnimSequence which is bound to a AnimBP
	//The editor will try to close all the windows, including this one, even though it is pending the AssetRegistryChangedEvent
	if (AnimBPEditor.IsValid())
	{
		FilteredSequences = AnimBPEditor.Pin()->AnimBPBeingEdited->GetAnimSequences();
		AnimSequenceList->RequestRefresh();
	}
}

TSharedRef<SWidget> SPaperZDMySequences::CreateAddNewMenuWidget()
{
	const bool bShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder MenuBuilder(bShouldCloseWindowAfterMenuSelection, AnimBPEditor.Pin()->GetToolkitCommands());

	BuildAddNewMenu(MenuBuilder);

	return MenuBuilder.MakeWidget();
}

void SPaperZDMySequences::BuildAddNewMenu(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.BeginSection("AddNewItem", LOCTEXT("AddOperations", "Add New"));
	{
		MenuBuilder.AddMenuEntry(FPaperZDAnimBPEditorCommands::Get().CreateAnimSequence);
	}
	MenuBuilder.EndSection();
}

TArray<UPaperZDAnimSequence*> SPaperZDMySequences::GetAnimSequences()
{
	return FilteredSequences;
}

void SPaperZDMySequences::HandleAnimSequenceSelected(UPaperZDAnimSequence *Sequence, ESelectInfo::Type SelectionType)
{
	//It can give a null if you click on the list, but not on some AnimSequence
	if (Sequence)
	{
		AnimBPEditor.Pin()->GetInspector()->ShowDetailsForSingleObject(Sequence);
		AnimBPEditor.Pin()->OpenDocument(Sequence, FDocumentTracker::EOpenDocumentCause::OpenNewDocument);
	}
}

TSharedPtr<SWidget> SPaperZDMySequences::GetListContextMenu()
{
	if (!AnimBPEditor.IsValid())
	{
		return TSharedPtr<SWidget>();
	}

	static const bool bShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder MenuBuilder(bShouldCloseWindowAfterMenuSelection, AnimBPEditor.Pin()->GetToolkitCommands());
	if (AnimSequenceList->GetNumItemsSelected() > 0)
	{
		MenuBuilder.BeginSection("EditSequence", LOCTEXT("EditSequence", "Edit"));
		MenuBuilder.AddMenuEntry(FPaperZDAnimBPEditorCommands::Get().RenameAnimSequence);
		MenuBuilder.AddMenuEntry(FPaperZDAnimBPEditorCommands::Get().DuplicateAnimSequence);
		MenuBuilder.AddMenuEntry(FPaperZDAnimBPEditorCommands::Get().DeleteAnimSequence);
		MenuBuilder.EndSection();
	}
	else
	{
		MenuBuilder.BeginSection("AddNewItem", LOCTEXT("AddOperations", "Add New"));
		MenuBuilder.AddMenuEntry(FPaperZDAnimBPEditorCommands::Get().CreateAnimSequence);
		MenuBuilder.EndSection();
	}

	return MenuBuilder.MakeWidget();
}

void SPaperZDMySequences::HandleCreateAnimSequence()
{
	UPaperZDAnimSequenceFactory* AnimSequenceFactory = NewObject<UPaperZDAnimSequenceFactory>(GetTransientPackage());
	// This factory may get gc'd as a side effect of various delegates potentially calling CollectGarbage so protect against it from being gc'd out from under us
	AnimSequenceFactory->AddToRoot();
	AnimSequenceFactory->TargetAnimBP = AnimBPEditor.Pin()->AnimBPBeingEdited;

	//Load asset tools and try to create the AnimSequence
	FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	UObject* CreatedAsset = nullptr;

	//Here we change how we create the asset, depending on user settings
	UPaperZDEditorSettings* Settings = CastChecked<UPaperZDEditorSettings>(UPaperZDEditorSettings::StaticClass()->ClassDefaultObject);
	if (Settings->SequencePlacementPolicy == EAnimSequencePlacementPolicy::AlwaysAsk)
	{
		//We should ask where to put the newly created AnimSequence (a simple call will suffice)
		CreatedAsset = AssetToolsModule.Get().CreateAssetWithDialog(UPaperZDAnimSequence::StaticClass(), AnimSequenceFactory, FName("AnimBPEditor_NewSequence"));
	}
	else
	{
		const FString DefaultAssetName = AnimSequenceFactory->GetDefaultNewAssetName();
		const FString AnimBPPackageName = AnimSequenceFactory->TargetAnimBP->GetOutermost()->GetName();
		FString AssetPath;

		if (Settings->SequencePlacementPolicy == EAnimSequencePlacementPolicy::SameFolder)
		{
			//We use the same folder as the path
			AssetPath = FPackageName::GetLongPackagePath(AnimBPPackageName);
		}
		else
		{
			AssetPath = FPackageName::GetLongPackagePath(AnimBPPackageName) + TEXT("/") + Settings->SequencePlacementFolderName.ToString();
		}

		//We will save the asset now, it won't prompt the configuration of the factories by itself, so make sure we do it manually
		if (AnimSequenceFactory->ConfigureProperties())
		{
			FString OutAssetName;
			FString OutPackageName;
			AssetToolsModule.Get().CreateUniqueAssetName(AssetPath + TEXT("/") + DefaultAssetName, TEXT(""), OutPackageName, OutAssetName);
			CreatedAsset = AssetToolsModule.Get().CreateAsset(OutAssetName, FPackageName::GetLongPackagePath(OutPackageName), UPaperZDAnimSequence::StaticClass(), AnimSequenceFactory, FName("AnimBPEditor_NewSequence"));
		}
	}

	//Cast and show the newly created sequence
	if (CreatedAsset)
	{
		ResetSequenceList();

		//Mark the selection, this will trigger the delegates and configure the view, as if the user clicked the item
		UPaperZDAnimSequence* CreatedSequence = Cast<UPaperZDAnimSequence>(CreatedAsset);
		AnimSequenceList->SetSelection(CreatedSequence);
	}

	//Remember to remove the factory
	AnimSequenceFactory->RemoveFromRoot();
}

void SPaperZDMySequences::HandleDuplicateSelectedAnimSequence()
{
	if (AnimSequenceList->GetNumItemsSelected())
	{
		TArray<UObject*> SelectedObjects;
		for (UPaperZDAnimSequence* Sequence : AnimSequenceList->GetSelectedItems())
		{
			SelectedObjects.Add(Sequence);
		}

		ObjectTools::DuplicateObjects(SelectedObjects, TEXT(""), TEXT(""), false);
		ResetSequenceList();
	}
}

void SPaperZDMySequences::HandleDeleteSelectedAnimSequence()
{
	if (AnimSequenceList->GetNumItemsSelected())
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		TArray<UObject*> SelectedObjects;
		for (UPaperZDAnimSequence* Sequence : AnimSequenceList->GetSelectedItems())
		{
			SelectedObjects.Add(Sequence);
		}

		//Handle deletion
		ObjectTools::DeleteObjects(SelectedObjects);
	}
}

void SPaperZDMySequences::HandleRenameSelectedAnimSequence()
{
	//Only support renaming one sequence
	if (AnimSequenceList->GetNumItemsSelected() == 1)
	{
		UPaperZDAnimSequence* SelectedSequence = AnimSequenceList->GetSelectedItems()[0];
		AnimSequenceList->EnterRenameMode(SelectedSequence);
	}
}

void SPaperZDMySequences::HandleAnimSequenceFocusRequest(UPaperZDAnimSequence* Sequence)
{
	if (Sequence)
	{
		AnimSequenceList->SetSelection(Sequence);
	}
}

#undef LOCTEXT_NAMESPACE
