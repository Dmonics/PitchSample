// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "Notifies/PaperZDAnimNotifyCustom.h"
#include "PaperZD.h"

UPaperZDAnimNotifyCustom::UPaperZDAnimNotifyCustom(const FObjectInitializer& ObjectInitializer)
	: Super()
{
}

void UPaperZDAnimNotifyCustom::OnReceiveNotify_Implementation(UPaperZDAnimInstance *OwningInstance /* = nullptr*/)
{
	//Owning instance can be null on editor
	if (OwningInstance)
	{
		//Will need to obtain the real function name on that instance (we hold the display name, but due to possible duplicated functions the name could vary)
		FName* FunctionNamePtr = OwningInstance->NotifyFunctionMap.Find(Name);
		UFunction* BoundFunction = nullptr;
		if (FunctionNamePtr)
		{
			BoundFunction = OwningInstance->FindFunction(*FunctionNamePtr);
		}

		//Call the bound function
		if (ensure(BoundFunction)) {
			//Create Buffer and call function
			uint8 *Buffer = (uint8*)FMemory_Alloca(BoundFunction->ParmsSize);
			FMemory::Memzero(Buffer, BoundFunction->ParmsSize);
			OwningInstance->ProcessEvent(BoundFunction, Buffer);
		}
	}
}
