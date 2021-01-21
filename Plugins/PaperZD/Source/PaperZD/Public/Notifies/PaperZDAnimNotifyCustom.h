// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once
#include "Notifies/PaperZDAnimNotify.h"
#include "PaperZDAnimNotifyCustom.generated.h"

UCLASS()
class PAPERZD_API UPaperZDAnimNotifyCustom : public UPaperZDAnimNotify
{
	GENERATED_UCLASS_BODY()
			
public:
	//Override the native notify implementation
	void OnReceiveNotify_Implementation(UPaperZDAnimInstance* OwningInstance = nullptr) override;
};
