// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "PaperZDAnimBP.h"
#include "PaperZD.h"
#include "AnimNodes/PaperZDAnimNode.h"
#include "PaperZDCustomVersion.h"
#include "PaperZDCharacter.h"
#include "AnimSequences/PaperZDAnimSequence.h"

#if WITH_EDITOR
#include "AnimNodes/PaperZDAnimNode_State.h"
#include "AssetRegistryModule.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphSchema.h"
#include "UObject/UObjectIterator.h"
#include "PaperZDCharacter.h"
#endif

#if WITH_EDITOR
TSharedPtr<IPaperZDEditorProxy> UPaperZDAnimBP::EditorProxy = nullptr;
const FString UPaperZDAnimBP::SequenceNameTemplate("NewAnimSequence");
#endif

UPaperZDAnimBP::UPaperZDAnimBP(const FObjectInitializer& ObjectInitializer)
	: Super()
{
	bRecompileOnLoad = true;
}

void UPaperZDAnimBP::PostLoad()
{
	Super::PostLoad();

	const int32 ZDVersion = GetLinkerCustomVersion(FPaperZDCustomVersion::GUID);

#if WITH_EDITOR
	if (ZDVersion < FPaperZDCustomVersion::AnimSequencesAdded)
	{
		//AnimSequences will be created, this is only for sanity
		AnimSequences_DEPRECATED.Empty();

		//To update the AnimBP to use AnimSequences we need info that's editor only (namely GraphNodes) to be updated, we need to use the proxy for this kind of operations
		//as the runtime module has no dependency on the editor module (and cannot have one)
		GetEditorProxy()->UpdateVersionToAnimSequences(this);
	}
	
	if (ZDVersion < FPaperZDCustomVersion::AnimNodeOuterFix)
	{
		//We need to use the proxy for this too, AnimGraphs must be updated
		GetEditorProxy()->UpdateVersionToAnimNodeOuterFix(this);
	}

	if (ZDVersion < FPaperZDCustomVersion::AnimSequenceCategoryAdded)
	{
		//We use the proxy so the updates are all on the proxy (the AnimBP class ends up being leaner)
		GetEditorProxy()->UpdateVersionToAnimSequenceCategoryAdded(this);
	}

	if (ZDVersion < FPaperZDCustomVersion::AnimSequenceAsStandaloneAsset)
	{
		//Call the proxy for updating version
		GetEditorProxy()->UpdateVersionToAnimSequenceAsStandaloneAsset(this);
	}
#endif
}

void UPaperZDAnimBP::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	Ar.UsingCustomVersion(FPaperZDCustomVersion::GUID);
}

#if WITH_EDITOR

void UPaperZDAnimBP::PostInitProperties()
{
	Super::PostInitProperties();
	if (!HasAnyFlags(RF_ClassDefaultObject | RF_NeedLoad))
	{
		CreateGraph();
	}
}

void UPaperZDAnimBP::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);

	//Need to handle specific cases that are defined on editor only settings
	GetEditorProxy()->HandleDuplicateAnimBP(this);
}

void UPaperZDAnimBP::UnregisterCustomNotify(const FName& NotifyName)
{
	//Delete the notify from the array
	RegisteredNotifyNames.Remove(NotifyName);
}

bool UPaperZDAnimBP::RegisterCustomNotify(const FName& NotifyName)
{
	if (!RegisteredNotifyNames.Contains(NotifyName))
	{
		RegisteredNotifyNames.Add(NotifyName);
		return true;
	}

	return false;
}
#endif // WITH_EDITOR

#if WITH_EDITOR
void UPaperZDAnimBP::SetupAnimNode(UPaperZDAnimNode *InAnimNode, bool bSelectNewNode)
{
	check(InAnimNode->GraphNode == NULL);

	UPaperZDAnimBP::GetEditorProxy()->SetupAnimNode(AnimationGraph, InAnimNode, bSelectNewNode);
}

UEdGraph* UPaperZDAnimBP::GetGraph()
{
	return AnimationGraph;
}

void UPaperZDAnimBP::CreateGraph()
{
	if (AnimationGraph == nullptr)
	{
		AnimationGraph = UPaperZDAnimBP::GetEditorProxy()->CreateNewAnimationGraph(this);

		// Give the schema a chance to fill out any required nodes (like the entry node or results node)
		const UEdGraphSchema* Schema = AnimationGraph->GetSchema();
		Schema->CreateDefaultNodesForGraph(*AnimationGraph);
		AnimationGraph->bAllowDeletion = false;
		
		//@TODO: Ver porque no se mantiene el grafico en el arreglo de last edited documents
		LastEditedDocuments.Add(AnimationGraph);
	}
}

void UPaperZDAnimBP::SetAnimationEditor(TSharedPtr<IPaperZDEditorProxy> InAnimationEditor)
{
	check(!EditorProxy.IsValid());
	EditorProxy = InAnimationEditor;
}

//Gets the editor implementation
TSharedPtr<IPaperZDEditorProxy> UPaperZDAnimBP::GetEditorProxy()
{
	return EditorProxy;
}

//@DEPRECATED
UPaperZDAnimSequence* UPaperZDAnimBP::CreateAnimSequence()
{
	UPaperZDAnimSequence *Seq = NewObject<UPaperZDAnimSequence>(this);
	Seq->Identifier = Seq->GetFName();

	//Generate a Unique Display Name por this sequence
	Seq->DisplayName_DEPRECATED = CreateUniqueSequenceDisplayName();

	//Add to the ordered array
	AnimSequences_DEPRECATED.Add(Seq);

	return Seq;
}

//@DEPRECATED
FName UPaperZDAnimBP::CreateUniqueSequenceDisplayName() const
{
	//Create a set to determine the existance of the name when testing
	TSet<FName> ExistingNames;

	for (UPaperZDAnimSequence* Seq : AnimSequences_DEPRECATED)
	{
		ExistingNames.Add(Seq->GetSequenceName());
	}

	//Iterate on each possible name, incrementing the appended string by one when not unique
	FString TestString;
	for (int32 i = 0; i < 30; i++) //Limit the search by 30
	{
		FString AppendString = FString::Printf(TEXT("_%d"), i);
		TestString = i == 0 ? SequenceNameTemplate : SequenceNameTemplate + AppendString;

		if (!ExistingNames.Contains(*TestString))
			break;
	}

	return *TestString;
}

TArray<class UPaperZDAnimSequence*> UPaperZDAnimBP::GetAnimSequences() const
{
	//@PATCH: just after duplication, the class constructor is invalid, the compile will then be run like 3 times before it is valid
	//We need the generated class to be valid in order for the AnimSequence getter to work and not crash, this here makes sure we're not in an invalid state
	if (GeneratedClass && GeneratedClass->ClassConstructor)
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		TArray<FAssetData> AssetData;

		//Create a filter, we only need AnimSequences that have this AnimBP as parent
		FARFilter Filter;
		Filter.ClassNames.Add(UPaperZDAnimSequence::StaticClass()->GetFName());
		Filter.bRecursiveClasses = true;
		Filter.TagsAndValues.Add(UPaperZDAnimSequence::GetAnimBPMemberName(), FAssetData(this).GetExportTextName());
		AssetRegistryModule.Get().GetAssets(Filter, AssetData);

		//Load the Sequences (could be unloaded for any reason), we need internal info
		TArray<UPaperZDAnimSequence*>Sequences;
		for (FAssetData Data : AssetData)
		{
			//Do a check on validity on AnimSequence, in cases like deleting from the ContentBrowser, deleted sequences will not be marked as RF_Standalone
			UPaperZDAnimSequence* Sequence = Cast<UPaperZDAnimSequence>(Data.GetAsset());
			if (Sequence->HasAllFlags(RF_Standalone))
			{
				Sequences.Add(Sequence);
			}
		}

		return Sequences;
	}
	else
	{
		return TArray<UPaperZDAnimSequence*>();
	}
}


void UPaperZDAnimBP::OnPreCompile()
{
	int32 index;
	UbergraphPages.Find(AnimationGraph, index);
	if (index != INDEX_NONE)
	{
		UbergraphPages.RemoveAt(index);
	}
}

void UPaperZDAnimBP::OnPostCompile()
{
	if (UbergraphPages.Find(AnimationGraph) == INDEX_NONE)
	{
		UbergraphPages.Add(AnimationGraph);
	}

	//Search for Characters using this AnimBP and notify them
	for (TObjectIterator<APaperZDCharacter> Itr; Itr; ++Itr)
	{
		// Access the subclass instance with the * or -> operators.
		APaperZDCharacter* Character = *Itr;

		if (this == Character->AnimationBlueprint)
			Character->OnAnimBPCompiled();
	}
	
	//Mark as dirty, because if the transition function names do change, on reload they will not have been saved, but the new function will have been recompiled, giving error and forcing a manual recompile
	MarkPackageDirty();
}
#endif
