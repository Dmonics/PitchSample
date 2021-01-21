// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "PaperZDAnimBPEditor.h"
#include "PaperZDEditor.h"
#include "PaperZDAnimInstance.h"
#include "PaperZDAnimBP.h"
#include "Toolkits/IToolkit.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/DebuggerCommands.h"
#include "SKismetInspector.h"
#include "AssetRegistryModule.h"
#include "AnimSequences/PaperZDAnimSequence.h"
#include "Editor.h"

#include "PaperZDAnimGraphNode_State.h"
#include "AnimNodes/PaperZDAnimNode_State.h"

#include "Util/PaperZDTabSpawners.h"
#include "Util/PaperZDAnimBPEditorCommands.h"

//Slate
#include "GraphEditor.h"
#include "GraphEditorActions.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/SBoxPanel.h"

#include "SBlueprintEditorToolbar.h"
#include "BlueprintEditorModes.h"
#include "SPaperZDConfirmDialog.h"

#include "Slate/SPaperZDModeSelectorWidget.h"

//Styles
#include "EditorStyleSet.h"

#include "Framework/Docking/TabManager.h"
#include "BlueprintEditorTabs.h"

//Modes
#include "WorkflowOrientedApp/ApplicationMode.h"

#define LOCTEXT_NAMESPACE "PaperZDAnimBPEditor"

const FName FPaperZDAnimBPEditor::PaperZDEditor_AnimationMode(TEXT("PaperZDEditor_AnimationMode"));
const FName FPaperZDAnimBPEditor::GraphCanvasTabId(TEXT("PaperZDAnimBPEditor_GraphCanvas"));
const FName FPaperZDAnimBPEditor::PropertiesTabId(TEXT("PaperZDAnimBPEditor_Properties"));
const FName FPaperZDAnimBPEditor::StateEditorTabId(TEXT("PaperZDAnimBPEditor_State"));

//////////////////////////////////////////////////////////////////////////
//// Modes
//////////////////////////////////////////////////////////////////////////
class FPaperZDAnimationMode : public FApplicationMode
{
private:
	// Set of spawnable tabs in this mode
	FWorkflowAllowedTabSet ComponentsTabFactories;


public:
	TWeakPtr<FPaperZDAnimBPEditor>AnimBPEditor;

	static FText GetLocalizedMode(const FName InMode)
	{
		return NSLOCTEXT("PaperZDEditor", "AnimationMode", "Animation");
	}

	FPaperZDAnimationMode(TSharedPtr<FPaperZDAnimBPEditor> InEditor) : FApplicationMode(FPaperZDAnimBPEditor::PaperZDEditor_AnimationMode,GetLocalizedMode)
	{
		AnimBPEditor = InEditor;

		ComponentsTabFactories.RegisterFactory(MakeShareable(new FSelectionDetailsSummoner(InEditor)));
		ComponentsTabFactories.RegisterFactory(MakeShareable(new FPaperZDMySequencesTabSummoner(InEditor)));

		//Menues and toolbars
		InEditor->GetToolbarBuilder()->AddCompileToolbar(ToolbarExtender);
		InEditor->GetToolbarBuilder()->AddScriptingToolbar(ToolbarExtender);
		InEditor->GetToolbarBuilder()->AddBlueprintGlobalOptionsToolbar(ToolbarExtender);
		InEditor->GetToolbarBuilder()->AddDebuggingToolbar(ToolbarExtender);

		//Create the tab layout
		TabLayout = FTabManager::NewLayout("PaperZDAnimBPLayout")
			->AddArea
			(
				FTabManager::NewPrimaryArea()
				->SetOrientation(Orient_Horizontal)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.15f)
					->AddTab(FPaperZDTabs::MySequencesTabID, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewSplitter()
					->SetOrientation(Orient_Vertical)
					->SetSizeCoefficient(0.6f)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.2f)
						->SetHideTabWell(true)
						->AddTab(InEditor->GetToolbarTabId(), ETabState::OpenedTab)
					)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.8f)
						->AddTab("Document", ETabState::ClosedTab)
					)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.25f)
					->AddTab(FBlueprintEditorTabs::DetailsID, ETabState::OpenedTab)
				)
			);
	}

	void RegisterTabFactories(TSharedPtr<FTabManager> InTabManager)
	{
		//Register the toolbar
		TSharedPtr<FPaperZDAnimBPEditor> Editor = AnimBPEditor.Pin();
		Editor->RegisterToolbarTab(InTabManager.ToSharedRef());

		//Custom tabs for this mode
		Editor->PushTabFactories(ComponentsTabFactories);
	}
};

//////////////////////////////////////////////////////////////////////////
//// PaperZD AnimBP Editor
//////////////////////////////////////////////////////////////////////////

FPaperZDAnimBPEditor::FPaperZDAnimBPEditor() : AnimBPBeingEdited(nullptr)
{
	//Bind delegates
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetRegistryModule.Get().OnAssetRemoved().AddRaw(this, &FPaperZDAnimBPEditor::OnAssetRemoved);
}

FPaperZDAnimBPEditor::~FPaperZDAnimBPEditor()
{
	//Unbind delegates
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetRegistryModule.Get().OnAssetRemoved().RemoveAll(this);
}

//FBlueprintEditor
void FPaperZDAnimBPEditor::JumpToHyperlink(const UObject* ObjectReference, bool bRequestRename)
{
	FBlueprintEditor::JumpToHyperlink(ObjectReference, bRequestRename);
	
	if (const UPaperZDAnimGraphNode_State* Node = Cast<const UPaperZDAnimGraphNode_State>(ObjectReference))
	{
		// Open the document
		UPaperZDAnimSequence *Sequence = Node->AnimSequence;
		if (Sequence)
		{
			OpenDocument(Sequence, FDocumentTracker::EOpenDocumentCause::OpenNewDocument);
		}
		else
		{
			TSharedRef<SPaperZDConfirmDialog> Dialog = SNew(SPaperZDConfirmDialog)
				.TitleText(FText::FromString(TEXT("Warning")))
				.DetailText(FText::FromString(TEXT("No valid AnimSequence set, please add an existing sequence or create one in the animation mode")))
				.ShowCancelButton(false);

			Dialog->Show();
		}
	}
}
//End of FBlueprintEditor

//FGCObject Interface
void FPaperZDAnimBPEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(AnimBPBeingEdited);
}
//END FGCObject Interface

//FAssetEditorToolkit
FName FPaperZDAnimBPEditor::GetToolkitFName() const
{
	return FName("PaperZDAnimBPEditor");
}

FText FPaperZDAnimBPEditor::GetBaseToolkitName() const
{
	return LOCTEXT("PaperZDAnimBPEditorAppLabel", "PaperZD AnimBP Editor");
}

FString FPaperZDAnimBPEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("PaperZD AnimBPEditor");
}

FText FPaperZDAnimBPEditor::GetToolkitName() const
{
	return FText::FromString(AnimBPBeingEdited->GetName());
}

FText FPaperZDAnimBPEditor::GetToolkitToolTipText() const
{
	return FAssetEditorToolkit::GetToolTipTextForObject(AnimBPBeingEdited);
}

FLinearColor FPaperZDAnimBPEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}
//End FAssetEditorToolkit

void FPaperZDAnimBPEditor::InitAnimBPEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost >& InitToolkitHost, UPaperZDAnimBP* InitAnimBP)
{
	AnimBPBeingEdited = InitAnimBP;
	
	if (!Toolbar.IsValid())
	{
		Toolbar = MakeShareable(new FBlueprintEditorToolbar(SharedThis(this)));
	}

	//Don't forget to append the world commands, the play button and others won't do shit if not connected
	GetToolkitCommands()->Append(FPlayWorldCommands::GlobalPlayWorldActions.ToSharedRef());

	//Support Undo
	GEditor->RegisterForUndo(this);

	//Commands
	FGraphEditorCommands::Register();
	FPaperZDAnimBPEditorCommands::Register();

	const TSharedRef<FTabManager::FLayout> DummyLayout = FTabManager::NewLayout("NullLayout")->AddArea(FTabManager::NewPrimaryArea());
	
	//Init the asset editor window
	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;
	FAssetEditorToolkit::InitAssetEditor(Mode, InitToolkitHost, TEXT("PaperZDAnimBPEditorApp"), DummyLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, InitAnimBP, false);

	//Common initialize the blueprint
	TArray<UBlueprint*> AnimBlueprints;
	AnimBlueprints.Add(InitAnimBP);

	//Must do
	CommonInitialization(AnimBlueprints);

	ExtendToolbar();
	RegenerateMenusAndToolbars();
	
	//Make sure the BlueprintMode register itself
	RegisterApplicationModes(AnimBlueprints, false);

	//Register our custom animation mode
	AddApplicationMode(FPaperZDAnimBPEditor::PaperZDEditor_AnimationMode, MakeShareable(new FPaperZDAnimationMode(SharedThis(this))));

	//Register the document factories
	TSharedPtr<FPaperZDAnimBPEditor> ThisPtr(SharedThis(this));
	FAnimSequenceDocumentTabSummoner *Summoner = new FAnimSequenceDocumentTabSummoner(ThisPtr, AnimBPBeingEdited);
	DocumentManager->RegisterDocumentFactory(MakeShareable(Summoner));

	//Some bullshit here
	PostLayoutBlueprintEditorInitialization();		
}

void FPaperZDAnimBPEditor::InitAnimBPEditor(const EToolkitMode::Type Mode, const TSharedPtr<class IToolkitHost>& InitToolkitHost, class UPaperZDAnimSequence* InAnimSequence)
{
	//First, make sure our AnimSequence is parented to an AnimBP.
	if (InAnimSequence->GetAnimBP())
	{
		//Initialize using the other signature method
		InitAnimBPEditor(Mode, InitToolkitHost, InAnimSequence->GetAnimBP());
		
		//Configure the view
		SetCurrentMode(FPaperZDAnimBPEditor::PaperZDEditor_AnimationMode);
		OnSequenceFocusRequest.ExecuteIfBound(InAnimSequence);
	}
}

void FPaperZDAnimBPEditor::ExtendToolbar()
{
	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
	ToolbarExtender->AddToolBarExtension(
		"Debugging",
		EExtensionHook::After,
		NULL,
		FToolBarExtensionDelegate::CreateLambda([this](FToolBarBuilder& ToolbarBuilder) 
		{

			TArray<FChangeModeInfo> ChangeModes;
			ChangeModes.Add(FChangeModeInfo(FBlueprintEditorApplicationModes::StandardBlueprintEditorMode.ToString(), "Blueprint",FText::FromString("Edit the blueprint logic and StateMachine"),"PaperZDEditor.ModeSwitcher.Blueprint"));
			ChangeModes.Add(FChangeModeInfo(FPaperZDAnimBPEditor::PaperZDEditor_AnimationMode.ToString(), "Animations", FText::FromString("Modify the Sequences that will be used no the AnimBP"), "PaperZDEditor.ModeSwitcher.Animation"));

			AddToolbarWidget(SNew(SPaperZDModeSelectorWidget, ChangeModes)
				.OnModeSelected(FOnModeSelected::CreateSP(this,&FPaperZDAnimBPEditor::OnModeSelected))
				.OnGetCurrentSectionIdentifier(FOnGetCurrentSectionIdentifier::CreateSP(this, &FPaperZDAnimBPEditor::GetSectionIdentifier))
			);
		}));

	//Add the extender
	AddToolbarExtender(ToolbarExtender);
}

void FPaperZDAnimBPEditor::OnModeSelected(FString Identifier)
{
	//The identifier holds the current mode to change
	SetCurrentMode(*Identifier);
}

FString FPaperZDAnimBPEditor::GetSectionIdentifier()
{
	return (IsModeCurrent(FPaperZDAnimBPEditor::PaperZDEditor_AnimationMode) ? FPaperZDAnimBPEditor::PaperZDEditor_AnimationMode : FBlueprintEditorApplicationModes::StandardBlueprintEditorMode).ToString();
}

void FPaperZDAnimBPEditor::OnAssetRemoved(const FAssetData& InAssetData)
{
	const UClass* AssetClass = InAssetData.GetClass();
	if (AssetClass && AssetClass->IsChildOf(UPaperZDAnimSequence::StaticClass()))
	{
		//Must close the document tab (if it corresponds to an open one, but validation happens inside the method, so we don't worry about it)
		//The referenced asset could be null, most likely due to a renaming operation, in which case the GetAsset method won't be able to find the referenced asset (due to it changing name)
		//In this case we do nothing, because our pointers are still valid
		UObject* ReferencedAsset = InAssetData.GetAsset();
		if (ReferencedAsset)
		{
			CloseDocumentTab(ReferencedAsset);
		}
	}
}

//Listener to use when selection changes
void FPaperZDAnimBPEditor::OnNotifySelectionChanged(TArray<UObject*> SelectedObjects)
{
	Inspector->ShowDetailsForObjects(SelectedObjects);
}

void FPaperZDAnimBPEditor::RegisterDefaultEventNodes()
{
	//For now we register using the static class, as there is only one default node
	FKismetEditorUtilities::RegisterAutoGeneratedDefaultEvent(UPaperZDAnimInstance::StaticClass(), UPaperZDAnimInstance::StaticClass(), FName(TEXT("OnTick")));
	FKismetEditorUtilities::RegisterAutoGeneratedDefaultEvent(UPaperZDAnimInstance::StaticClass(), UPaperZDAnimInstance::StaticClass(), FName(TEXT("OnInit")));
}

void FPaperZDAnimBPEditor::UnregisterDefaultEventNodes()
{
	FKismetEditorUtilities::UnregisterAutoBlueprintNodeCreation(UPaperZDAnimInstance::StaticClass());
}
#undef LOCTEXT_NAMESPACE
