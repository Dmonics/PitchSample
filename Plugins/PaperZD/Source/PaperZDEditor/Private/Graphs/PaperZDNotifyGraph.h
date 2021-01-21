// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "EdGraph/EdGraph.h"
#include "PaperZDNotifyGraph.generated.h"


UCLASS()
class UPaperZDNotifyGraph : public UEdGraph
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY()
	FName NotifyName;
};
