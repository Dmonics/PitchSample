// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "PaperZDEditor.h"
#include "Editors/PaperZDAnimBPEditor.h"
#include "AssetTypeActions/AssetTypeActions_PaperZDAnimBP.h"
#include "AssetTypeActions/AssetTypeActions_ZDAnimSeq.h"
#include "PaperZDCompilerContext.h"
#include "PaperZDAnimBP.h"
#include "PaperZDCharacter.h"
#include "Editors/Util/PaperZDRuntimeEditorProxy.h"
#include "Editors/Util/PaperZDEditorStyle.h"
#include "KismetCompiler.h"
#include "PropertyEditorModule.h"

//Custom Settings
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "ISettingsContainer.h"
#include "Editors/Util/PaperZDEditorSettings.h"

//Detail Customization
#include "PaperZDAnimGraphNode_State.h"
#include "PaperZDAnimGraphNode_Transition.h"
#include "Editors/DetailCustomizations/AnimGraphNodeDetailCustomization.h"

//For Sequencer support
#include "ISequencerModule.h"
#include "PaperZDAnimationTrackEditor.h"

#define LOCTEXT_NAMESPACE "FPaperZDEditorModule"
void FPaperZDEditorModule::StartupModule()
{
	RegisterAssetActions();
	RegisterCompiler();
	
	// Register the TrackEditor for 2d sequencer animation
	ISequencerModule& SequencerModule = FModuleManager::LoadModuleChecked<ISequencerModule>("Sequencer");
	SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FPaperZDAnimationTrackEditor::CreateTrackEditor));

	//Register the custom settings
	RegisterSettings();
	
	//Register Default Nodes for the editor
	FPaperZDAnimBPEditor::RegisterDefaultEventNodes();

	//Register Detail Customizations
	RegisterCustomizations();

	//Register the editor style
	FPaperZDEditorStyle::Initialize();

	//Finally register the EditorProxy, so the runtime part can configure editor-only functionalities when this module is up and running
	FPaperZDRuntimeEditorProxy::Register();
}

void FPaperZDEditorModule::ShutdownModule()
{
	UnregisterAssetActions();

	// UnRegister the compiler for the ZD AnimBP
	if (FModuleManager::Get().IsModuleLoaded("KismetCompiler")) //@TODO: check because it fails on 4.17
	{
		IKismetCompilerInterface& KismetCompilerModule = FModuleManager::LoadModuleChecked<IKismetCompilerInterface>("KismetCompiler");
		KismetCompilerModule.GetCompilers().Remove(this);
	}

	FPaperZDAnimBPEditor::UnregisterDefaultEventNodes();
}

//////////////////////////////////////////////////////////////////////////
//// IBlueprintCompiler Methods
//////////////////////////////////////////////////////////////////////////
bool FPaperZDEditorModule::CanCompile(const UBlueprint* Blueprint)
{
	return Cast<UPaperZDAnimBP>(Blueprint) != nullptr;
}

void FPaperZDEditorModule::Compile(UBlueprint* Blueprint, const FKismetCompilerOptions& CompileOptions, FCompilerResultsLog& Results)
{
	UPaperZDAnimBP *AnimBP = Cast<UPaperZDAnimBP>(Blueprint);
	check(AnimBP);

	FPaperZDCompilerContext Compiler(Blueprint, Results, CompileOptions);
	Compiler.Compile();

	//Mark as dirty, because if the transition function names do change, on reload they will not have been saved, but the new function will have been recompiled, giving error and forcing a manual recompile
	AnimBP->MarkPackageDirty();
}

//////////////////////////////////////////////////////////////////////////
//// PaperZDEditor  Methods
//////////////////////////////////////////////////////////////////////////
//Blueprint Compiler Methods
void FPaperZDEditorModule::RegisterCompiler()
{
	// Register the compiler for the ZD AnimBP - Old Method (without blueprint compilation manager)
	IKismetCompilerInterface& KismetCompilerModule = FModuleManager::LoadModuleChecked<IKismetCompilerInterface>("KismetCompiler");
	KismetCompilerModule.GetCompilers().Add(this);

	//Register for blueprint compilation manager
	FKismetCompilerContext::RegisterCompilerForBP(UPaperZDAnimBP::StaticClass(), &FPaperZDEditorModule::GetCompilerForAnimBP);

	//Register Pre/Post Compilation
}

void FPaperZDEditorModule::RegisterSettings()
{
	// Register custom editor settings
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule)
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "PaperZDEditor",
			LOCTEXT("EditorSettingsName", "PaperZD (Editor)"),
			LOCTEXT("EditorSettingsDescription", "Configure common PaperZD settings"),
			GetMutableDefault<UPaperZDEditorSettings>());
	}
}

void FPaperZDEditorModule::UnregisterSettings()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule)
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "PaperZDEditor");
	}
}

TSharedPtr<FKismetCompilerContext> FPaperZDEditorModule::GetCompilerForAnimBP(UBlueprint* BP, FCompilerResultsLog& InMessageLog, const FKismetCompilerOptions& InCompileOptions)
{
	return TSharedPtr<FKismetCompilerContext>(new FPaperZDCompilerContext(BP, InMessageLog, InCompileOptions));
}

void FPaperZDEditorModule::RegisterAssetActions()
{
	// Register the ZD Editor asset type actions
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	EAssetTypeCategories::Type CustomType = AssetTools.RegisterAdvancedAssetCategory(FAssetTypeActions_PaperZDAnimBP::CategoryKey, FAssetTypeActions_PaperZDAnimBP::CategoryDisplayName);

	AnimBPAssetTypeActions = MakeShareable(new FAssetTypeActions_PaperZDAnimBP(CustomType));
	AssetTools.RegisterAssetTypeActions(AnimBPAssetTypeActions->AsShared());

	AnimSequenceAssetTypeActions = MakeShareable(new FAssetTypeActions_ZDAnimSeq(CustomType));
	AssetTools.RegisterAssetTypeActions(AnimSequenceAssetTypeActions->AsShared());
}

void FPaperZDEditorModule::UnregisterAssetActions()
{
	// Register the ZD Editor asset type actions
	if (FModuleManager::Get().IsModuleLoaded("AssetTools")) //@TODO: this only fails on 4.17
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		AssetTools.UnregisterAssetTypeActions(AnimBPAssetTypeActions->AsShared());
		AssetTools.UnregisterAssetTypeActions(AnimSequenceAssetTypeActions->AsShared());

		AnimBPAssetTypeActions = nullptr;
	}
}

void FPaperZDEditorModule::RegisterCustomizations()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	//Custom detail views
	PropertyModule.RegisterCustomClassLayout(UPaperZDAnimGraphNode_State::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FAnimGraphNodeDetailCustomization::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(UPaperZDAnimGraphNode_Transition::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FAnimGraphNodeDetailCustomization::MakeInstance));
}

//////////////////////////////////////////////////////////////////////////
//// AnimBP Editor
//////////////////////////////////////////////////////////////////////////
TSharedRef<FPaperZDAnimBPEditor> FPaperZDEditorModule::CreateAnimBPEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, class UPaperZDAnimBP* InitAnimBP)
{
	TSharedRef<FPaperZDAnimBPEditor> NewAnimBPEditor(new FPaperZDAnimBPEditor());
	NewAnimBPEditor->InitAnimBPEditor(Mode, InitToolkitHost, InitAnimBP);
	return NewAnimBPEditor;
}

TSharedRef<FPaperZDAnimBPEditor> FPaperZDEditorModule::CreateAnimBPEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, class UPaperZDAnimSequence* InAnimSequence)
{
	TSharedRef<FPaperZDAnimBPEditor> NewAnimBPEditor(new FPaperZDAnimBPEditor());
	NewAnimBPEditor->InitAnimBPEditor(Mode, InitToolkitHost, InAnimSequence);
	return NewAnimBPEditor;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPaperZDEditorModule, PaperZDEditor)
