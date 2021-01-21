// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once
#include "Modules/ModuleManager.h"
#include "KismetCompilerModule.h"
#include "Toolkits/IToolkitHost.h"

class SGraphEditor;
class FPaperZDAnimBPEditor;
class FPaperZDEditorModule : public IModuleInterface, public IBlueprintCompiler
{
public:
	TSharedPtr<SGraphEditor> AnimationGraphEditor;

private:
	TSharedPtr<class FAssetTypeActions_PaperZDAnimBP> AnimBPAssetTypeActions;
	TSharedPtr<class FAssetTypeActions_ZDAnimSeq> AnimSequenceAssetTypeActions;
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//End IModuleInterface

	//IBlueprintCompiler Interface
	virtual bool CanCompile(const UBlueprint* Blueprint) override;
	virtual void Compile(UBlueprint* Blueprint, const FKismetCompilerOptions& CompileOptions, FCompilerResultsLog& Results) override;
	//End of IBlueprintCompiler Interface

	//For BlueprintCompilationManager
	void RegisterCompiler();
	static TSharedPtr<class FKismetCompilerContext> GetCompilerForAnimBP(UBlueprint* BP, FCompilerResultsLog& InMessageLog, const FKismetCompilerOptions& InCompileOptions);

	//Registry
	virtual void RegisterAssetActions();
	virtual void UnregisterAssetActions();
	virtual void RegisterCustomizations();
	
	
	//Create and return a AnimBPEditor Instance
	TSharedRef<FPaperZDAnimBPEditor> CreateAnimBPEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, class UPaperZDAnimBP* InitAnimBP);

	//Create and return a AnimBPEditor Instance
	TSharedRef<FPaperZDAnimBPEditor> CreateAnimBPEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, class UPaperZDAnimSequence* InAnimSequence);

private:
	void RegisterSettings();
	void UnregisterSettings();
};
