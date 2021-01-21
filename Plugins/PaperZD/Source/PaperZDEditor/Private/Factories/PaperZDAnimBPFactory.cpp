// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "PaperZDAnimBPFactory.h"
#include "PaperZDEditor.h"
#include "PaperZDAnimInstance.h"
#include "Kismet2/KismetEditorUtilities.h"



#define LOCTEXT_NAMESPACE "PaperZDAnimBlueprintFactory"

UPaperZDAnimBPFactory::UPaperZDAnimBPFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UPaperZDAnimBP::StaticClass();
	ParentClass = UPaperZDAnimInstance::StaticClass();
	BlueprintType = EBlueprintType::BPTYPE_Normal;
}

UObject* UPaperZDAnimBPFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{

	UPaperZDAnimBP* NewBP = CastChecked<UPaperZDAnimBP>(FKismetEditorUtilities::CreateBlueprint(ParentClass, InParent, Name, BlueprintType, UPaperZDAnimBP::StaticClass(), UBlueprintGeneratedClass::StaticClass()));
	FKismetEditorUtilities::CompileBlueprint(NewBP);

	return NewBP;
}

#undef LOCTEXT_NAMESPACE