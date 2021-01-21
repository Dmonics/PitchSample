// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "AnimNodes/PaperZDAnimNode_TransBase.h"
#include "PaperZD.h"
//#include "CoreUObject.h"


UPaperZDAnimNode_TransBase::UPaperZDAnimNode_TransBase(const FObjectInitializer& ObjectInitializer)
	: Super()
{
	Type = EPaperZDNodeType::NODE_Transition;
}

void UPaperZDAnimNode_TransBase::Init(UPaperZDAnimInstance *OwningInstance)
{
	BoundFunction = OwningInstance->FindFunction(FunctionName);
}

bool UPaperZDAnimNode_TransBase::CanEnter(UPaperZDAnimInstance* OwningInstance, TSet<const UPaperZDAnimNode*>& VisitedNodes) const
{
	bool bCanEnter = Super::CanEnter(OwningInstance, VisitedNodes);

	//First make sure we clear any cached target node
	CachedTarget = TWeakObjectPtr<UPaperZDAnimNode>();

	//First early out when we have no function or we already came full circle on the search
	if (ensure(BoundFunction) && bCanEnter)
	{
		//Check update the function and check it
		{
			//Create Buffer and call function
			uint8 *Buffer = (uint8*)FMemory_Alloca(BoundFunction->ParmsSize);
			FMemory::Memzero(Buffer, BoundFunction->ParmsSize);
			OwningInstance->ProcessEvent(BoundFunction, Buffer);

			//Obtain the return value (Out Parameters)
			for (TFieldIterator<UProperty> PropIt(BoundFunction, EFieldIteratorFlags::ExcludeSuper); PropIt; ++PropIt)
			{
				UProperty* Property = *PropIt;
				if (Property->HasAnyPropertyFlags(CPF_OutParm)) {
					uint8* outValueAddr = Property->ContainerPtrToValuePtr<uint8>(Buffer);
					bool* pReturn = (bool*)outValueAddr;
					bCanEnter = *pReturn;
					break;
				}
			}
		}

		//Early out, we don't want to waste time checking target nodes if we already fail
		if (!bCanEnter)
			return false;

		//Obtain the possible nodes and check if they can be entered
		TArray<UPaperZDAnimNode *>TargetNodes = GetOutputNodes();
		for (UPaperZDAnimNode *Target : TargetNodes)
		{
			if (Target->CanEnter(OwningInstance, VisitedNodes))
			{
				//Found a target that we can land to, we don't need to keep searching
				CachedTarget = Target;
				return true;
			}
		}
	}

	return false;
}

void UPaperZDAnimNode_TransBase::Enter(UPaperZDAnimInstance *OwningInstance)
{
	//We should already have a cached state, just enter it
	if (!ensure(CachedTarget.IsValid()))
		return;

	CachedTarget->Enter(OwningInstance);
}
