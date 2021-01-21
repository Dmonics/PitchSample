// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "PaperZDCompilerContext.h"
#include "PaperZDAnimBP.h"
#include "PaperZDEditor.h"
#include "PaperZDAnimInstance.h"
#include "PaperZDTransitionGraphNode_Result.h"
#include "PaperZDAnimTransitionGraph.h"
#include "EdGraphUtilities.h"
#include "AnimSequences/PaperZDAnimSequence.h"

//Graph Nodes
#include "PaperZDAnimGraphNode_Root.h"
#include "PaperZDAnimGraphNode_State.h"
#include "PaperZDAnimGraphNode_Transition.h"
#include "PaperZDAnimGraphNode_Jump.h"

//Anim Nodes
#include "AnimNodes/PaperZDAnimNode_State.h"
#include "AnimNodes/PaperZDAnimNode_Transition.h"
#include "AnimNodes/PaperZDAnimNode_Root.h"
#include "AnimNodes/PaperZDAnimNode_Jump.h"

//K2 Nodes
#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"

#include "Kismet2/BlueprintEditorUtils.h"
#include "PaperZDNotifyGraph.h"
#include "Notifies/PaperZDAnimNotify.h"
#include "Notifies/PaperZDAnimNotifyCustom.h"

void FPaperZDCompilerContext::CreateFunctionList()
{
	//Create the Notify Functions, they get added to the FunctionGraph list so they appear on the editor too
	ConfigureNotifyFunctionGraphs();


	//Obtain the transition nodes and expand the graphs
	TArray<UPaperZDAnimGraphNode_TransBase *> TransitionNodes;
	AnimBP->GetGraph()->GetNodesOfClass(TransitionNodes);
	for (auto It = TransitionNodes.CreateIterator(); It; ++It)
	{
		UPaperZDAnimGraphNode_TransBase *Node = *It;
		AnimBP->FunctionGraphs.Add(TranslateTransitionNodeToFunctionGraph(Node));
	}

	//Call parent
	FKismetCompilerContext::CreateFunctionList();
}

void FPaperZDCompilerContext::ConfigureNotifyFunctionGraphs()
{
	TSet<FName> VisitedNotifies;
	//First delete any function graph that doesn't belong here anymore
	for (int i = AnimBP->FunctionGraphs.Num() - 1; i >= 0; i--)
	{
		UPaperZDNotifyGraph *NotifyGraph = Cast<UPaperZDNotifyGraph>(AnimBP->FunctionGraphs[i]);
		if (NotifyGraph)
		{
			//Check if the notify still exists (hasn't been deleted)
			int index = AnimBP->RegisteredNotifyNames.Find(NotifyGraph->NotifyName); 

			if (index == INDEX_NONE) {
				AnimBP->FunctionGraphs.RemoveAt(i);
			}
			else {
				VisitedNotifies.Add(NotifyGraph->NotifyName); //Found, save it so we don't create it again

				//Add to the Mapping of Notify-FunctionName
				TArray<UK2Node_FunctionEntry*> EntryPoints;
				NotifyGraph->GetNodesOfClass(EntryPoints);

				//Only 1 entry point allowed
				check(EntryPoints.Num() == 1);

				NotifyFunctionNameMap.Add(NotifyGraph->NotifyName, EntryPoints[0]->CustomGeneratedFunctionName);
			}
		}		
	}

	//Create any notify graph that's new
	for (FName name : AnimBP->RegisteredNotifyNames)
	{
		if (VisitedNotifies.Contains(name)) //Skip if already visited on previous steps
			continue;

		//Create the function graph
		FName FunctionName = GetUniqueFunctionName(FString(TEXT("ReceiveNotify_")).Append(name.ToString()));
		UPaperZDNotifyGraph *Graph = CastChecked<UPaperZDNotifyGraph>(FBlueprintEditorUtils::CreateNewGraph(Blueprint, FunctionName, UPaperZDNotifyGraph::StaticClass(), UEdGraphSchema_K2::StaticClass())); 
		Graph->bAllowDeletion = false;
		Graph->NotifyName = name;
		
		FGraphNodeCreator<UK2Node_FunctionEntry> NodeCreator(*Graph);
		UK2Node_FunctionEntry* FunctionEntry = NodeCreator.CreateNode(false);
		FunctionEntry->CustomGeneratedFunctionName = FunctionName;
		FunctionEntry->AllocateDefaultPins();
		
		//Commit the node
		NodeCreator.Finalize();

		//Finally add to the graph
		AnimBP->FunctionGraphs.Add(Graph);

		//And register the name of the function
		NotifyFunctionNameMap.Add(name, FunctionEntry->CustomGeneratedFunctionName);
	}
}

UEdGraph* FPaperZDCompilerContext::TranslateTransitionNodeToFunctionGraph(UPaperZDAnimGraphNode_TransBase *TransitionNode)
{
	UPaperZDAnimTransitionGraph* ClonedGraph = CastChecked<UPaperZDAnimTransitionGraph>(FEdGraphUtilities::CloneGraph(TransitionNode->GetBoundGraph(), Blueprint, &MessageLog, true));
	const bool bIsLoading = Blueprint->bIsRegeneratingOnLoad || IsAsyncLoading();

	//Get result Node
	UPaperZDTransitionGraphNode_Result *ResultNode = ClonedGraph->GetResultNode();

	//Create the event Node
	FName UniqueName = GetUniqueFunctionName(TEXT("PaperZD_TransitionFunction"));
	UK2Node_FunctionEntry* EntryNode = SpawnIntermediateNode<UK2Node_FunctionEntry>(ResultNode, ClonedGraph);
	EntryNode->CustomGeneratedFunctionName = UniqueName;
	EntryNode->AllocateDefaultPins();

	//Create the bool reference pin
	UEdGraphPin *TransitionResultPin = ResultNode->Pins[0]; //@TODO: GETTER
	FEdGraphPinType Type = TransitionResultPin->PinType;

	//Create the function output
	UK2Node_FunctionResult* ReturnNode = SpawnIntermediateNode<UK2Node_FunctionResult>(ResultNode, ClonedGraph);
	ReturnNode->AllocateDefaultPins();

	//Connect the entry to the return
	UEdGraphPin* ExecVariablesIn = CastChecked<UEdGraphSchema_K2>(ClonedGraph->GetSchema())->FindExecutionPin(*EntryNode, EGPD_Output);
	ReturnNode->GetExecPin()->MakeLinkTo(ExecVariablesIn);

	//Create the pin
	UEdGraphPin *ReturnPin = ReturnNode->CreatePin(EGPD_Input,Type,TransitionResultPin->GetFName());
	ReturnPin->DefaultValue = TransitionResultPin->DefaultValue;

	//Check if the result node has any connections or is using the default value only (no logic or getter connected)
	if (TransitionResultPin->LinkedTo.Num())
	{
		//Should only be connected to one node
		check(TransitionResultPin->LinkedTo.Num() == 1);

		UEdGraphPin *ValuePin = TransitionResultPin->LinkedTo[0];
		ValuePin->MakeLinkTo(ReturnPin);
	}

	//Clear the node and remove it
	ResultNode->BreakAllNodeLinks();
	ClonedGraph->RemoveNode(ResultNode);

	//Store the function name
	CastChecked<UPaperZDAnimNode_TransBase>(TransitionNode->GetAnimNode())->FunctionName = UniqueName;

	return ClonedGraph;
}

void FPaperZDCompilerContext::PreCompile()
{
	AnimBP->OnPreCompile(); 
}

void FPaperZDCompilerContext::PostCompile()
{
	AnimBP->OnPostCompile();

	//Clear any Notifies from each sequence, that use old custom notifies names
	for (auto Seq : AnimBP->GetAnimSequences())
	{
		Seq->CleanInvalidNodes(AnimBP->RegisteredNotifyNames);
	}

	//Remove transition Graphs (so they don't show on the editor)
	for (int i = Blueprint->FunctionGraphs.Num() - 1; i >= 0; i--)
	{
		if (Cast<UPaperZDAnimTransitionGraph>(Blueprint->FunctionGraphs[i]))
		{
			Blueprint->FunctionGraphs.RemoveAt(i);
		}
	}
}

void FPaperZDCompilerContext::CopyTermDefaultsToDefaultObject(UObject* DefaultObject)
{
	FKismetCompilerContext::CopyTermDefaultsToDefaultObject(DefaultObject);

	UPaperZDAnimInstance* GeneratedInstance = CastChecked<UPaperZDAnimInstance>(DefaultObject);
	GeneratedInstance->NotifyFunctionMap = NotifyFunctionNameMap;

	//Create the state machine and then copy the base nodes
	GenerateStateMachine(GeneratedInstance);
	GeneratedInstance->RootNode = StateMachineRoot;
	GeneratedInstance->JumpNodeMap = JumpNodeMap;
}

FName FPaperZDCompilerContext::GetUniqueFunctionName(FString PrependString)
{
	int32 j = 0;
	FString TestString;
	//FString PrependWithNumber = PrependString + "_%d";
	while (true)
	{
		TestString = j == 0 ? PrependString : PrependString + FString::Printf(TEXT("_%d"), j);
		/*if (j == 0)
			TestString = PrependString;
		else
			TestString = FString::Printf(*PrependWithNumber, j);*/

		bool bUnique = true;
		for (int i = 0; i < AnimBP->FunctionGraphs.Num(); i++) {
			TArray<UK2Node_FunctionEntry*> EntryPoints;
			AnimBP->FunctionGraphs[i]->GetNodesOfClass(EntryPoints);


			//Should only be an entry point
			check(EntryPoints.Num() == 1);
			bUnique = !EntryPoints[0]->CustomGeneratedFunctionName.IsEqual(FName(*TestString));

			if (!bUnique)
				break;
		}

		if (bUnique)
			return FName(*TestString);
		else
			j++;
	}
}

void FPaperZDCompilerContext::GenerateStateMachine(UPaperZDAnimInstance *GeneratedInstance)
{
	//Obtain the transition nodes and expand the graphs
	TArray<UPaperZDAnimGraphNode_Root *> RootNodes;
	AnimBP->GetGraph()->GetNodesOfClass(RootNodes);

	//Should only have one root node
	check(RootNodes.Num() == 1);

	//Start with the Root node
	StateMachineRoot = CastChecked<UPaperZDAnimNode_Root>(Recursive_ProcessGraphNode(RootNodes[0], GeneratedInstance));

	//Optionally continue with the jump nodes
	TArray<UPaperZDAnimGraphNode_Jump *> JumpNodes;
	AnimBP->GetGraph()->GetNodesOfClass(JumpNodes);
	for (UPaperZDAnimGraphNode_Jump* JumpGraphNode : JumpNodes)
	{
		UPaperZDAnimNode_Jump* JumpRuntimeNode = CastChecked<UPaperZDAnimNode_Jump>(Recursive_ProcessGraphNode(JumpGraphNode, GeneratedInstance));
		JumpNodeMap.Add(JumpGraphNode->GetJumpName(), JumpRuntimeNode);
	}
}

UPaperZDAnimNode* FPaperZDCompilerContext::Recursive_ProcessGraphNode(UPaperZDAnimGraphNode* GraphNode, UPaperZDAnimInstance* GeneratedInstance)
{
	//Check if this node was already visited
	if (VisitedNodes.Contains(GraphNode))
		return VisitedNodes[GraphNode];

	UPaperZDAnimNode *RuntimeNode = DuplicateObject(GraphNode->GetAnimNode(), GeneratedInstance); //@TODO: We shouldn't really be duplicating an anim node, but creating one here
	RuntimeNode->AnimInstance = GeneratedInstance;

	//Add the node before visiting another (could come to a redundant cycle before adding the node), we
	VisitedNodes.Add(GraphNode, RuntimeNode);

	//First try to configure the state nodes
	if (UPaperZDAnimGraphNode_State *l = Cast<UPaperZDAnimGraphNode_State>(GraphNode))
	{
		UPaperZDAnimNode_State *StateNode = Cast<UPaperZDAnimNode_State>(RuntimeNode);
		StateNode->AnimSequence = l->AnimSequence;

		//Store config variables
		StateNode->bShouldLoop = l->bShouldLoop;
	}
	else if (UPaperZDAnimGraphNode_Transition *t = Cast<UPaperZDAnimGraphNode_Transition>(GraphNode))
	{
		UPaperZDAnimNode_Transition *TransitionNode = Cast<UPaperZDAnimNode_Transition>(RuntimeNode);
		TransitionNode->AnimSequence= t->AnimSequence;

		//Store config variables
		TransitionNode->Priority = t->Priority;
	}

	//Recursively connect the nodes
	TArray<UPaperZDAnimNode *>ConnectedNodes;
	for (int i = 0; i < GraphNode->GetOutputPin()->LinkedTo.Num(); i++)
	{
		UPaperZDAnimGraphNode *LinkedNode = CastChecked<UPaperZDAnimGraphNode>(GraphNode->GetOutputPin()->LinkedTo[i]->GetOwningNode());
		ConnectedNodes.Add(Recursive_ProcessGraphNode(LinkedNode, GeneratedInstance));
	}

	//Connect the output of the runtime node with the connected nodes
	RuntimeNode->ConnectOutputWith(ConnectedNodes);
	return RuntimeNode;
}
