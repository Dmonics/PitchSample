// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once
#include "KismetCompiler.h"

class UPaperZDAnimNode_Jump;
class FPaperZDCompilerContext : public FKismetCompilerContext
{
private:
	class UPaperZDAnimBP *AnimBP;
	class UPaperZDAnimNode_Root *StateMachineRoot;
	TMap<class UPaperZDAnimGraphNode*, class UPaperZDAnimNode*> VisitedNodes;
	TMap<FName, FName> NotifyFunctionNameMap;
	TSet<FName>RegisteredZDFunctionNames;
	TMap<FName, UPaperZDAnimNode_Jump*> JumpNodeMap;

public:
	FPaperZDCompilerContext(UBlueprint* Blueprint, FCompilerResultsLog& InMessageLog, const FKismetCompilerOptions& InCompilerOptions) :
		FKismetCompilerContext(Blueprint, InMessageLog, InCompilerOptions),
		AnimBP(CastChecked<UPaperZDAnimBP>(Blueprint))
	{
	}

	//~FKismetCompilerContext
	virtual void PreCompile() override;
	virtual void CopyTermDefaultsToDefaultObject(UObject* DefaultObject) override;
	virtual void PostCompile() override;
	virtual void CreateFunctionList() override;
	//~FKismetCompilerContext

private:
	UEdGraph* TranslateTransitionNodeToFunctionGraph(class UPaperZDAnimGraphNode_TransBase *TransitionNode);
	FName GetUniqueFunctionName(FString PrependString);
	void GenerateStateMachine(class UPaperZDAnimInstance *GeneratedInstance);
	class UPaperZDAnimNode* Recursive_ProcessGraphNode(class UPaperZDAnimGraphNode* GraphNode, class UPaperZDAnimInstance *GeneratedInstance);
	void ConfigureNotifyFunctionGraphs();
};
