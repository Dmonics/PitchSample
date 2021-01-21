// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "PaperZDRuntimeEditorProxy.h"
#include "PaperZDEditorSettings.h"
#include "AssetToolsModule.h"
#include "Factories/PaperZDAnimSequenceFactory.h"

#include "AnimSequences/PaperZDAnimSequence_Flipbook.h"
#include "Notifies/PaperZDAnimNotify_Base.h"
#include "AnimNodes/PaperZDAnimNode_State.h"
#include "PaperZDAnimGraphSchema.h"
#include "PaperZDAnimGraph.h"
#include "Kismet2/BlueprintEditorUtils.h"

//Nodes
#include "PaperZDAnimGraphNode_State.h"
#include "PaperZDAnimGraphNode_Transition.h"
#include "PaperZDAnimGraphNode_Conduit.h"
#include "PaperZDAnimGraphNode_Root.h"
#include "PaperZDAnimGraphNode_Jump.h"

void FPaperZDRuntimeEditorProxy::Register()
{
	if (!UPaperZDAnimBP::GetEditorProxy().IsValid())
	{
		UPaperZDAnimBP::SetAnimationEditor(TSharedPtr<IPaperZDEditorProxy>(new FPaperZDRuntimeEditorProxy));
	}
}

UEdGraph* FPaperZDRuntimeEditorProxy::CreateNewAnimationGraph(UPaperZDAnimBP* InAnimBP)
{
	UPaperZDAnimGraph* AnimGraph = CastChecked<UPaperZDAnimGraph>(FBlueprintEditorUtils::CreateNewGraph(InAnimBP, NAME_None, UPaperZDAnimGraph::StaticClass(), UPaperZDAnimGraphSchema::StaticClass()));
	return Cast<UEdGraph>(AnimGraph);
}

void FPaperZDRuntimeEditorProxy::SetupAnimNode(UEdGraph* AnimGraph, UPaperZDAnimNode *InAnimNode, bool bSelectNewNode)
{
	UPaperZDAnimGraphNode *GraphNode;
	switch (InAnimNode->Type)
	{
	case EPaperZDNodeType::NODE_Root:
		GraphNode = CreateRootNode(AnimGraph, InAnimNode, bSelectNewNode);
		break;
	case EPaperZDNodeType::NODE_State:
		GraphNode = CreateStateNode(AnimGraph, InAnimNode, bSelectNewNode);
		break;
	case EPaperZDNodeType::NODE_Transition:
		GraphNode = CreateTransitionNode(AnimGraph, InAnimNode, bSelectNewNode);
		break;
	case EPaperZDNodeType::NODE_Conduit:
		GraphNode = CreateConduitNode(AnimGraph, InAnimNode, bSelectNewNode);
		break;
	case EPaperZDNodeType::NODE_Jump:
		GraphNode = CreateJumpNode(AnimGraph, InAnimNode, bSelectNewNode);
		break;
	default:
		return;
	}
}

UPaperZDAnimGraphNode* FPaperZDRuntimeEditorProxy::CreateRootNode(UEdGraph* AnimGraph, UPaperZDAnimNode *InAnimNode, bool bSelectNewNode)
{
	FGraphNodeCreator<UPaperZDAnimGraphNode_Root> NodeCreator(*AnimGraph);
	UPaperZDAnimGraphNode* GraphNode = NodeCreator.CreateNode(bSelectNewNode);
	GraphNode->SetAnimNode(InAnimNode);
	NodeCreator.Finalize();

	return GraphNode;
}

UPaperZDAnimGraphNode* FPaperZDRuntimeEditorProxy::CreateStateNode(UEdGraph* AnimGraph, UPaperZDAnimNode *InAnimNode, bool bSelectNewNode)
{
	FGraphNodeCreator<UPaperZDAnimGraphNode_State> NodeCreator(*AnimGraph);
	UPaperZDAnimGraphNode* GraphNode = NodeCreator.CreateNode(bSelectNewNode);
	GraphNode->SetAnimNode(InAnimNode);
	NodeCreator.Finalize();

	return GraphNode;
}

UPaperZDAnimGraphNode* FPaperZDRuntimeEditorProxy::CreateTransitionNode(UEdGraph* AnimGraph, UPaperZDAnimNode *InAnimNode, bool bSelectNewNode)
{
	FGraphNodeCreator<UPaperZDAnimGraphNode_Transition> NodeCreator(*AnimGraph);
	UPaperZDAnimGraphNode* GraphNode = NodeCreator.CreateNode(bSelectNewNode);
	GraphNode->SetAnimNode(InAnimNode);
	NodeCreator.Finalize();

	return GraphNode;
}

UPaperZDAnimGraphNode* FPaperZDRuntimeEditorProxy::CreateConduitNode(UEdGraph* AnimGraph, UPaperZDAnimNode *InAnimNode, bool bSelectNewNode)
{
	FGraphNodeCreator<UPaperZDAnimGraphNode_Conduit> NodeCreator(*AnimGraph);
	UPaperZDAnimGraphNode* GraphNode = NodeCreator.CreateNode(bSelectNewNode);
	GraphNode->SetAnimNode(InAnimNode);
	NodeCreator.Finalize();

	return GraphNode;
}

UPaperZDAnimGraphNode* FPaperZDRuntimeEditorProxy::CreateJumpNode(UEdGraph* AnimGraph, UPaperZDAnimNode *InAnimNode, bool bSelectNewNode)
{
	FGraphNodeCreator<UPaperZDAnimGraphNode_Jump> NodeCreator(*AnimGraph);
	UPaperZDAnimGraphNode* GraphNode = NodeCreator.CreateNode(bSelectNewNode);
	GraphNode->SetAnimNode(InAnimNode);
	NodeCreator.Finalize();

	return GraphNode;
}

void FPaperZDRuntimeEditorProxy::HandleDuplicateAnimBP(UPaperZDAnimBP* InAnimBP)
{
	//After duplication, we need to operate on any State and Transition node that holds an AnimSequence and either remove the reference, or create
	//and assign a duplicated asset for this AnimBP, for this decision we use the settings object to know which option to use
	UPaperZDEditorSettings* Settings = UPaperZDEditorSettings::StaticClass()->GetDefaultObject<UPaperZDEditorSettings>();
	if (Settings)
	{
		//Asset tools is needed when we duplicate AnimSequences, will be loaded now so we do it only once, even if we end up not using it
		FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
		const FAssetData AnimBPAssetData = FAssetData(InAnimBP);
		const FString DuplicatePackagePath = AnimBPAssetData.PackagePath.ToString() + TEXT("/") + Settings->SequenceDuplicationFolderName.ToString();

		//Create a TMap that will hold a mapping of the old anim sequence to newly created anim sequence
		TMap<UPaperZDAnimSequence*, UPaperZDAnimSequence*> SequenceMapping;
	
		TArray<UPaperZDAnimGraphNode*> GraphNodes;
		InAnimBP->GetGraph()->GetNodesOfClass(GraphNodes);

		for (UPaperZDAnimGraphNode* Node : GraphNodes)
		{
			//Obtain the memory address of the AnimSequence, we only use this method to avoid duplicating the code below, could be done better with an interface for "playable" states
			UPaperZDAnimSequence** pAnimSequence = nullptr;
			if (UPaperZDAnimGraphNode_State* StateNode = Cast<UPaperZDAnimGraphNode_State>(Node))
			{
				pAnimSequence = &(StateNode->AnimSequence);
			}
			else if (UPaperZDAnimGraphNode_Transition* TransitionNode = Cast<UPaperZDAnimGraphNode_Transition>(Node))
			{
				pAnimSequence = &(TransitionNode->AnimSequence);
			}

			//Only work if we have a pointer to the anim sequence (and it points to a valid AnimSequence)
			if (pAnimSequence && *pAnimSequence)
			{
				if (Settings->DuplicationPolicy == EAnimBlueprintDuplicationPolicy::DuplicateOnlyWrapper)
				{
					//In this case, we won't duplicate any used AnimSequence, so we just nullptr every AnimSequence
					*pAnimSequence = nullptr;
				}
				else
				{
					//In this case, we need to create a new duplicated AnimSequence asset if there's a referenced AnimSequence, we map it so we just reuse any already duplicated sequence
					//First check we haven't already duplicated this sequence
					if (UPaperZDAnimSequence** FoundSequence = SequenceMapping.Find(*pAnimSequence))
					{
						*pAnimSequence = *FoundSequence;
					}
					else
					{
						//First time encountering this AnimSequence, we need to duplicate it
						const FAssetData AnimSequenceAssetData = FAssetData(*pAnimSequence);
						const FString PackagePath = AnimBPAssetData.PackagePath.ToString() + TEXT("/") + Settings->SequenceDuplicationFolderName.ToString();
						FString OutPackageName;
						FString OutAssetName;
						AssetToolsModule.Get().CreateUniqueAssetName(PackagePath + TEXT("/") + AnimSequenceAssetData.AssetName.ToString(), TEXT(""), OutPackageName, OutAssetName);
						UPaperZDAnimSequence* DuplicatedSequence = Cast<UPaperZDAnimSequence>(AssetToolsModule.Get().DuplicateAsset(OutAssetName, PackagePath, *pAnimSequence));

						if (DuplicatedSequence)
						{
							//Need to reparent this sequence to the new AnimBP and make sure it saves
							DuplicatedSequence->AnimBP = InAnimBP;
							DuplicatedSequence->MarkPackageDirty();

							//Add to the mapping for further use, and replace the sequence
							SequenceMapping.Add(*pAnimSequence, DuplicatedSequence);
							*pAnimSequence = DuplicatedSequence;
						}
					}
				}
			}
		}
	}
}

void FPaperZDRuntimeEditorProxy::UpdateVersionToAnimSequences(UPaperZDAnimBP *InAnimBP)
{
	//First obtain all the graph state nodes
	TArray<UPaperZDAnimGraphNode_State*> StateNodes;
	InAnimBP->AnimationGraph->GetNodesOfClass(StateNodes);

	for (UPaperZDAnimGraphNode_State *Node : StateNodes)
	{
		//If no flipbook is setup, there's no reason to update this node
		if (!Node->Flipbook_DEPRECATED)
			continue;
		
		UPaperZDAnimNode_State *RuntimeNode = Cast<UPaperZDAnimNode_State>(Node->GetAnimNode());

		//Create an anim sequence for this state
		UPaperZDAnimSequence *AnimSequence = InAnimBP->CreateAnimSequence(); //@Deprecated method

		//The identifier for old sequences is the name of the AnimNode
		AnimSequence->Identifier = RuntimeNode->GetFName();
		
		//Copy all other data
		AnimSequence->PaperFlipbook_DEPRECATED = Node->Flipbook_DEPRECATED;
		AnimSequence->DisplayName_DEPRECATED = Node->Flipbook_DEPRECATED->GetFName(); //Name will be the Flipbook name, until edited

		//Copy all the notifies
		AnimSequence->CachedAnimTracks.Empty();
		for (UPaperZDAnimTrack *Track : RuntimeNode->CachedAnimTracks_DEPRECATED) //Needed friend access for this to work "cleanly"
		{
			AnimSequence->CachedAnimTracks.Add(DuplicateObject<UPaperZDAnimTrack>(Track, AnimSequence));
		}

		AnimSequence->AnimNotifies.Empty();
		for (UPaperZDAnimNotify_Base *Notify: RuntimeNode->AnimNotifies_DEPRECATED) //Needed friend access for this to work "cleanly"
		{
			AnimSequence->AnimNotifies.Add(DuplicateObject<UPaperZDAnimNotify_Base>(Notify, AnimSequence));
		}

		//As the cached information has changed, we need to re-init the tracks and AnimNotifies
		AnimSequence->InitTracks();

		//Finally, assign this sequence to the GraphNode
		Node->SequenceIdentifier_DEPRECATED = AnimSequence->Identifier;
	}

	//As we forced new AnimSequences to the AnimBP, we should mark this as modified
	FBlueprintEditorUtils::MarkBlueprintAsModified(InAnimBP); 
}

void FPaperZDRuntimeEditorProxy::UpdateVersionToAnimNodeOuterFix(UPaperZDAnimBP *InAnimBP)
{
	//First obtain all the graph nodes
	TArray<UPaperZDAnimGraphNode*> Nodes;
	InAnimBP->AnimationGraph->GetNodesOfClass(Nodes);

	for (UPaperZDAnimGraphNode *GraphNode : Nodes)
	{
		//Reparent the runtime node to the correct hierarchy, this is done by duplication, because we cannot rename the node, due to it having PendingLoad as a flag still
		UPaperZDAnimNode *OldNode = GraphNode->GetAnimNode();
		UPaperZDAnimNode *RuntimeNode = DuplicateObject(OldNode, GraphNode);
		GraphNode->SetAnimNode(RuntimeNode);

		//Need to destroy the old dangling node, so it doesn't persist in memory
		OldNode->MarkPendingKill();
	}

	//Mark as modified, just to be sure
	FBlueprintEditorUtils::MarkBlueprintAsModified(InAnimBP);
}

void FPaperZDRuntimeEditorProxy::UpdateVersionToAnimSequenceCategoryAdded(UPaperZDAnimBP *InAnimBP)
{
	//Obtain all of the AnimSequences (Changed Getter to direct access to deprecated AnimSequences, as they do not point to the array anymore)
	TArray<UPaperZDAnimSequence*> AnimSequences = InAnimBP->AnimSequences_DEPRECATED;
	for (UPaperZDAnimSequence* Seq : AnimSequences)
	{
		Seq->Category = UPaperZDAnimSequence::DefaultCategory;
	}
}

void FPaperZDRuntimeEditorProxy::UpdateVersionToAnimSequenceAsStandaloneAsset(UPaperZDAnimBP* InAnimBP)
{
	//Creation of AnimSequences will be done accordingly to the creation policy of the settings object
	UPaperZDEditorSettings* Settings = UPaperZDEditorSettings::StaticClass()->GetDefaultObject<UPaperZDEditorSettings>();

	//Load and pre-configure the factory
	UPaperZDAnimSequenceFactory* Factory = NewObject<UPaperZDAnimSequenceFactory>(GetTransientPackage());
	Factory->TargetAnimBP = InAnimBP;
	Factory->AddToRoot();

	//Asset tools is needed when we duplicate AnimSequences, will be loaded now so we do it only once, even if we end up not using it
	FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	const FAssetData AnimBPAssetData = FAssetData(InAnimBP);

	//Update works in two stages, first we duplicate every existing AnimSequence into an asset and create a mapping array
	TMap<FName, UPaperZDAnimSequence_Flipbook*> SequenceMapping;

	//Changed Getter to direct access to deprecated AnimSequences, as they do not point to the array anymore
	TArray<UPaperZDAnimSequence*> AnimSequences = InAnimBP->AnimSequences_DEPRECATED;
	for (UPaperZDAnimSequence* Seq : AnimSequences)
	{
		//First time encountering this AnimSequence, we need to duplicate it
		const FString PackagePath = AnimBPAssetData.PackagePath.ToString() + TEXT("/") + Settings->SequencePlacementFolderName.ToString();
		const FString AssetName = Seq->DisplayName_DEPRECATED == NAME_None && Seq->PaperFlipbook_DEPRECATED != nullptr ? Seq->PaperFlipbook_DEPRECATED->GetName() : Seq->DisplayName_DEPRECATED.ToString();
		FString OutPackageName;
		FString OutAssetName;
		AssetToolsModule.Get().CreateUniqueAssetName(PackagePath + TEXT("/") + AssetName, TEXT(""), OutPackageName, OutAssetName);
		UObject* CreatedObject = AssetToolsModule.Get().CreateAsset(OutAssetName, PackagePath, UPaperZDAnimSequence_Flipbook::StaticClass(), Factory);

		if (UPaperZDAnimSequence_Flipbook* CreatedSequence = Cast<UPaperZDAnimSequence_Flipbook>(CreatedObject))
		{
			//Given that we cannot duplicate a base class sequence into the new AnimSequence_Flipbook, we will have to manually pass the corresponding data
			for (class UPaperZDAnimTrack* Track : Seq->CachedAnimTracks)
			{
				CreatedSequence->CachedAnimTracks.Add(DuplicateObject(Track, CreatedSequence));
			}

			for (class UPaperZDAnimNotify_Base* Notify : Seq->AnimNotifies)
			{
				CreatedSequence->AnimNotifies.Add(DuplicateObject(Notify, CreatedSequence));
			}

			//Even if the identifier is no longer in use, we will pass it so sequencer has a chance to recognize this identifier and recover some of the sequences
			CreatedSequence->Identifier = Seq->Identifier;
			CreatedSequence->Category = Seq->Category;

			//Finally pass the old deprecated flipbook on the base class, to the new specialized class
			CreatedSequence->Flipbook = Seq->PaperFlipbook_DEPRECATED;

			//Mark for saving
			CreatedSequence->GetOutermost()->SetDirtyFlag(true);

			//Add to the mapping for further use, and replace the sequence
			SequenceMapping.Add(CreatedSequence->Identifier, CreatedSequence);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Trying to migrate AnimSequence to asset failed for sequence: %s"), *Seq->GetName());
		}
	}

	//Factory no longer needed
	Factory->RemoveFromRoot();

	//Second stage consists on updating every graph node that contains an AnimSequence to the newly created standalone assets
	TArray<UPaperZDAnimGraphNode*> GraphNodes;
	InAnimBP->GetGraph()->GetNodesOfClass(GraphNodes);

	for (UPaperZDAnimGraphNode* Node : GraphNodes)
	{
		//Obtain the memory address of the AnimSequence, we only use this method to avoid duplicating the code below, could be done better with an interface for "playable" states
		UPaperZDAnimSequence** pAnimSequence = nullptr;
		FName SequenceIdentifier;
		if (UPaperZDAnimGraphNode_State* StateNode = Cast<UPaperZDAnimGraphNode_State>(Node))
		{
			pAnimSequence = &(StateNode->AnimSequence);
			SequenceIdentifier = StateNode->SequenceIdentifier_DEPRECATED;
		}
		else if (UPaperZDAnimGraphNode_Transition* TransitionNode = Cast<UPaperZDAnimGraphNode_Transition>(Node))
		{
			pAnimSequence = &(TransitionNode->AnimSequence);
			SequenceIdentifier = TransitionNode->SequenceIdentifier_DEPRECATED;
		}

		//Only work if we have a pointer to the anim sequence (and it isn't null)
		if (pAnimSequence && !SequenceIdentifier.IsNone())
		{
			//Check that the AnimSequence is indeed on the mapping (should be)
			if (UPaperZDAnimSequence_Flipbook** FoundSequence = SequenceMapping.Find(SequenceIdentifier))
			{
				*pAnimSequence = *FoundSequence;
			}
			else
			{
				//For some reason, we didn't create the Sequence that goes along this node, should notify that on log and clear out the sequence
				UE_LOG(LogTemp, Error, TEXT("Migrating graph node sequence on AnimBP %s failed, AnimSequence: %s"), *InAnimBP->GetName(),*(*pAnimSequence)->GetName());
				*pAnimSequence = nullptr;
			}
		}
	}

	//We should dirty the package, so we make sure it saves    
	InAnimBP->GetOutermost()->SetDirtyFlag(true);

	//Mark as modified, just to be sure
	FBlueprintEditorUtils::MarkBlueprintAsModified(InAnimBP);
}
