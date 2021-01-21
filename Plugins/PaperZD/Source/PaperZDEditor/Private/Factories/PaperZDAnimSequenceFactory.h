// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Factories/Factory.h"
#include "ClassViewerFilter.h"
#include "PaperZDAnimSequenceFactory.generated.h"

struct FPaperZDAnimSequenceFactoryClassFilter : IClassViewerFilter
{
	virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<class FClassViewerFilterFuncs> InFilterFuncs) override;

	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef<const IUnloadedBlueprintData> InUnloadedClassData, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override;
};

struct FAssetData;
class SWindow;

UCLASS()
class UPaperZDAnimSequenceFactory : public UFactory
{
	GENERATED_BODY()

private:
	/* Picked window Ptr */
	TSharedPtr<SWindow> PickerWindow;
		
public:
	UPROPERTY()
	class UPaperZDAnimBP* TargetAnimBP;

	UPROPERTY()
	/* Class to create when the factory needs to do so. */
	TSubclassOf<class UPaperZDAnimSequence> AnimSequenceClass;

public:
	UPaperZDAnimSequenceFactory(const FObjectInitializer& ObjectInitializer);
	
	//~ Begin UFactory Interface
	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ShouldShowInNewMenu() const override { return true; }
	//~ Begin UFactory Interface	

public:
	/* Lets the user choose from a list of available classes that inherit from the base ZDAnimSequence */
	bool PickAvailableClass(UClass*& PickedClass) const;

private:
	void OnTargetAnimBPSelected(const FAssetData& SelectedAsset);
};
