// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "UObject/GCObject.h"
#include "Toolkits/IToolkitHost.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "BlueprintEditor.h"

class UPaperZDAnimBP;
class SGraphEditor;
class IDetailsView;
class SDockTab;
class FTabInfo;
struct FAssetData;
class UPaperZDAnimSequence;

DECLARE_DELEGATE_OneParam(FOnSequenceFocusRequestSignature, UPaperZDAnimSequence*);
class FPaperZDAnimBPEditor : public FBlueprintEditor
{


public:
	TSharedPtr<SGraphEditor> GraphEditor;
	TSharedPtr<IDetailsView> DetailsView;
	UPaperZDAnimBP *AnimBPBeingEdited;

	static const FName PaperZDEditor_AnimationMode;
	static const FName GraphCanvasTabId;
	static const FName PropertiesTabId;
	static const FName StateEditorTabId;

public:
	FPaperZDAnimBPEditor();
	~FPaperZDAnimBPEditor();

	//FBlueprintEditor
	// Jumps to a hyperlinked node, pin, or graph, if it belongs to this blueprint
	virtual void JumpToHyperlink(const UObject* ObjectReference, bool bRequestRename) override;
	//End of FBlueprintEditor

	//FAssetEditorToolkit
	virtual FName GetToolkitFName() const override;				
	virtual FText GetBaseToolkitName() const override;			
	virtual FText GetToolkitName() const override;
	virtual FText GetToolkitToolTipText() const override;
	virtual FString GetWorldCentricTabPrefix() const override;	
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	//End FAssetEditorToolkit

	// FSerializableObject interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	// End of FSerializableObject interface

	/* Initializes the AnimBP Editor and enters the default AnimBP Blueprint Tab */
	void InitAnimBPEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, class UPaperZDAnimBP* InitAnimBP);

	/* Initializes the AnimBP Editor and immediately enters the Animation blueprint mode, selecting the given AnimSequence. */
	void InitAnimBPEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, class UPaperZDAnimSequence* InAnimSequence);

	/* Adds the ChangeMode toolbar to the normal blueprint Toolbar */
	void ExtendToolbar();

	/* Obtains the string identifier for the current mode. */
	FString GetSectionIdentifier();

	/* Called when the user selects one of the toolbar modes. */
	void OnModeSelected(FString Identifier);

	/* Registers OnTick/OnInit... on the AnimBP */
	static void RegisterDefaultEventNodes();

	/* Unregisters OnTick/OnInit... on the AnimBP */
	static void UnregisterDefaultEventNodes();

	//Listener to use when selection changes, public because it gets called from the TabSpawners (TODO: friend class)
	void OnNotifySelectionChanged(TArray<UObject*> SelectedObjects);

	/* 
	 *Delegate that gets called when we are asked to focus an AnimSequence externally, due to the fact that we do not have a direct pointer to the SMySequences slate object,
	 * we just broadcast the request and expect whoever is bound as a delegate to do the operation.
	 */
	FOnSequenceFocusRequestSignature OnSequenceFocusRequest;

private:
	/* Called when an asset is deleted, in case we have a document tab opened with that asset */
	void OnAssetRemoved(const FAssetData& InAssetData);

};
