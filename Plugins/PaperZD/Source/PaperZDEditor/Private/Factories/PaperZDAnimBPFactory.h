// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once
#include "Factories/Factory.h"
#include "PaperZDAnimBP.h"
#include "PaperZDAnimBPFactory.generated.h"

UCLASS()
class UPaperZDAnimBPFactory : public UFactory
{
	GENERATED_UCLASS_BODY()
public:
	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	// End of UFactory interface

	// The type of blueprint that will be created
	UPROPERTY(EditAnywhere, Category = "PaperZD Factory")
	TEnumAsByte<EBlueprintType> BlueprintType;

	// The parent class of the created blueprint
	UPROPERTY(EditAnywhere, Category = "PaperZD Factory", meta = (AllowAbstract = ""))
	TSubclassOf<UObject> ParentClass;
};
