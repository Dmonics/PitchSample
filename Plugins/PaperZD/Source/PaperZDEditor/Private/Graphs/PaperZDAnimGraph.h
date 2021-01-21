// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "EdGraph/EdGraph.h"
#include "PaperZDAnimGraph.generated.h"


UCLASS()
class UPaperZDAnimGraph : public UEdGraph
{
	GENERATED_UCLASS_BODY()

public:
	/** Returns the AnimBP that contains this graph */
	class UPaperZDAnimBP* GetAnimBP() const;
};
