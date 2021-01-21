// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "AssetTypeActions/AssetTypeActions_ZDAnimSeq.h"
#include "PaperZDEditor.h"
#include "AnimSequences/PaperZDAnimSequence.h"
#include "Toolkits/IToolkitHost.h"
#include "Editors/Util/PaperZDEditorStyle.h"
#include "Editors/Slate/SPaperZDConfirmDialog.h"
#include "Modules/ModuleManager.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "Widgets/SWindow.h"
#include "Widgets/Layout/SBorder.h"
#include "PaperZDAnimBP.h"
#include "Notifies/PaperZDAnimNotifyCustom.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Misc/ScopedSlowTask.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#include "Editor.h"

//Styles
#include "EditorStyleSet.h"

#define LOCTEXT_NAMESPACE "AssetTypeActionsPaperZDAnimSeq"

const FName FAssetTypeActions_ZDAnimSeq::CategoryKey = FName("PaperZD_AssetActionsKey");
const FText FAssetTypeActions_ZDAnimSeq::CategoryDisplayName = FText::FromString("PaperZetaD");

FAssetTypeActions_ZDAnimSeq::FAssetTypeActions_ZDAnimSeq(EAssetTypeCategories::Type InAssetCategory) :
MyAssetCategory(InAssetCategory)
{
}

UClass* FAssetTypeActions_ZDAnimSeq::GetSupportedClass() const
{
	return UPaperZDAnimSequence::StaticClass();
}

void FAssetTypeActions_ZDAnimSeq::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;
	FPaperZDEditorModule *EditorModule = &FModuleManager::LoadModuleChecked<FPaperZDEditorModule>("PaperZDEditor");

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		UPaperZDAnimSequence* Sequence = Cast<UPaperZDAnimSequence>(*ObjIt);
		if (Sequence && Sequence->GetAnimBP())
		{
			EditorModule->CreateAnimBPEditor(EToolkitMode::Standalone, EditWithinLevelEditor, Sequence);
		}
		else
		{
			//Not  a valid AnimBP parented to the Sequence, show error message
			TSharedRef<SPaperZDConfirmDialog> Dialog = SNew(SPaperZDConfirmDialog)
				.TitleText(FText::FromString(TEXT("Warning")))
				.DetailText(FText::FromString(TEXT("Selected AnimSequence isn't parented to a valid AnimBP, please reparent it!")))
				.ShowCancelButton(false);

			Dialog->Show();
		}
	}
}

uint32 FAssetTypeActions_ZDAnimSeq::GetCategories()
{
	return MyAssetCategory;
}

FText FAssetTypeActions_ZDAnimSeq::GetAssetDescription(const FAssetData& AssetData) const
{
	return FText::FromString("Contains render and notify data that can be used to drive a PaperZD Animation Blueprint");
}

bool FAssetTypeActions_ZDAnimSeq::HasActions(const TArray<UObject *>& InObjects) const
{
	return true;
}

void FAssetTypeActions_ZDAnimSeq::GetActions(const TArray<UObject *>& InObjects, FMenuBuilder& MenuBuilder)
{
	TArray<TWeakObjectPtr<UPaperZDAnimSequence>> AnimSequences = GetTypedWeakObjectPtrs<UPaperZDAnimSequence>(InObjects);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("AnimSequence_Reparent", "Reparent Sequence"),
		LOCTEXT("AnimSequence_ReparentTooltip", "Changes the AnimBP parent of this AnimSequence, registering all the custom notifies missing on the target."),
		//FSlateIcon(FEditorStyle::GetStyleSetName(), "ClassIcon.PaperFlipbook"),
		FSlateIcon(FPaperZDEditorStyle::GetStyleSetName(), "ClassThumbnail.PaperZDAnimBP"),
		FUIAction(
			FExecuteAction::CreateSP(this, &FAssetTypeActions_ZDAnimSeq::ExecuteReparentSequences, AnimSequences),
			FCanExecuteAction()
		)
	);
}

void FAssetTypeActions_ZDAnimSeq::ExecuteReparentSequences(TArray<TWeakObjectPtr<UPaperZDAnimSequence>> InSequences)
{
	//First we need the user to select the target AnimBP
	// Load the content browser module to display an asset picker
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	FAssetPickerConfig AssetPickerConfig;

	/** The asset picker will only show AnimBP */
	AssetPickerConfig.Filter.ClassNames.Add(UPaperZDAnimBP::StaticClass()->GetFName());
	AssetPickerConfig.Filter.bRecursiveClasses = true;

	/** The delegate that fires when an asset was selected */
	UPaperZDAnimBP* SelectedAnimBP;
	TSharedPtr<SWindow> ReparentPickerWindow;
	AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateLambda(
		[&SelectedAnimBP, &ReparentPickerWindow](const FAssetData SelectedAsset)
		{
			SelectedAnimBP = Cast<UPaperZDAnimBP>(SelectedAsset.GetAsset());
			ReparentPickerWindow->RequestDestroyWindow();
		}
	);

	/** The default view mode should be a list view */
	AssetPickerConfig.InitialAssetViewType = EAssetViewType::List;

	ReparentPickerWindow = SNew(SWindow)
		.Title(LOCTEXT("ReparentAnimSequenceOptions", "Pick Parent Animation Blueprint"))
		.ClientSize(FVector2D(500, 600))
		.SupportsMinimize(false).SupportsMaximize(false)
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("Menu.Background"))
		[
			ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
		]
		];

	GEditor->EditorAddModalWindow(ReparentPickerWindow.ToSharedRef());
	ReparentPickerWindow.Reset();

	//Actually perform the re-parenting
	if (SelectedAnimBP)
	{
		FScopedSlowTask SlowTask(InSequences.Num(), LOCTEXT("ImportSlowTask", "Importing"));
		SlowTask.MakeDialog();

		bool bAnimBPNeedsCompile = false;
		for (TWeakObjectPtr<UPaperZDAnimSequence> SequencePtr : InSequences)
		{
			if (SequencePtr.IsValid())
			{
				SlowTask.EnterProgressFrame(1, FText::Format(LOCTEXT("PaperZD_ReparentingProgress", "Reparenting \"{0}\"..."), FText::FromString(SequencePtr->GetName())));

				//No sense on applying changes if the sequence is already parented to the target animbp
				if (SequencePtr->GetAnimBP() != SelectedAnimBP)
				{
					TArray<UPaperZDAnimNotify_Base*> AnimNotifies = SequencePtr->GetAnimNotifies();
					for (UPaperZDAnimNotify_Base* Notify : AnimNotifies)
					{
						//We only care about the custom notifies, as they need a registered function on their parent AnimBP to actually work
						if (UPaperZDAnimNotifyCustom* CustomNotify = Cast<UPaperZDAnimNotifyCustom>(Notify))
						{
							//On custom notifies, the name points to the bound function identifier
							bAnimBPNeedsCompile |= SelectedAnimBP->RegisterCustomNotify(CustomNotify->Name);
						}
					}

					//Actually re-parent and set to save
					SequencePtr->SetAnimBP(SelectedAnimBP);
					SequencePtr->MarkPackageDirty();
				}
			}
		}

		if (bAnimBPNeedsCompile)
		{
			FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(SelectedAnimBP);
		}
	}
}

#undef LOCTEXT_NAMESPACE
