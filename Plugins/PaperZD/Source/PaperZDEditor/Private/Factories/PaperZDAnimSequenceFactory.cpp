// Fill out your copyright notice in the Description page of Project Settings.

#include "PaperZDAnimSequenceFactory.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SWindow.h"
#include "Widgets/Layout/SBorder.h"
#include "EditorStyleSet.h"
#include "PaperZDAnimBP.h"
#include "AnimSequences/PaperZDAnimSequence.h"
#include "Editor.h"
#include "Modules/ModuleManager.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"

//For class picker
#include "ClassViewerModule.h"
#include "Kismet2/SClassPickerDialog.h"

#define LOCTEXT_NAMESPACE "ZDAnimSequenceFactory_Flipbook"

//////////////////////////////////////////////////////////////////////////
//// Filter
//////////////////////////////////////////////////////////////////////////
bool FPaperZDAnimSequenceFactoryClassFilter::IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<class FClassViewerFilterFuncs> InFilterFuncs)
{
	TSet<const UClass*> ChildOfSet;
	ChildOfSet.Add(UPaperZDAnimSequence::StaticClass());

	return !InClass->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists) && InFilterFuncs->IfInChildOfClassesSet(ChildOfSet, InClass) != EFilterReturn::Failed;
}

bool FPaperZDAnimSequenceFactoryClassFilter::IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef<const IUnloadedBlueprintData> InUnloadedClassData, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs)
{
	TSet<const UClass*> ChildOfSet;
	ChildOfSet.Add(UPaperZDAnimSequence::StaticClass());

	return !InUnloadedClassData->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists) && InFilterFuncs->IfInChildOfClassesSet(ChildOfSet, InUnloadedClassData) != EFilterReturn::Failed;
}

//////////////////////////////////////////////////////////////////////////
//// Factory
//////////////////////////////////////////////////////////////////////////
UPaperZDAnimSequenceFactory::UPaperZDAnimSequenceFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	SupportedClass = UPaperZDAnimSequence::StaticClass();
}

bool UPaperZDAnimSequenceFactory::ConfigureProperties()
{
	if (AnimSequenceClass == nullptr)
	{
		UClass* PickedClass = nullptr;
		const bool bClassPicked = PickAvailableClass(PickedClass);

		if (bClassPicked)
		{
			AnimSequenceClass = PickedClass;
		}
		else
		{
			//No sense showing following dialog
			return false;
		}
	}

	// Optionally select the AnimBP if it hasn't been provided yet
	if (TargetAnimBP == nullptr)
	{
		// Load the content browser module to display an asset picker
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

		FAssetPickerConfig AssetPickerConfig;

		/** The asset picker will only show AnimBP */
		AssetPickerConfig.Filter.ClassNames.Add(UPaperZDAnimBP::StaticClass()->GetFName());
		AssetPickerConfig.Filter.bRecursiveClasses = true;

		/** The delegate that fires when an asset was selected */
		AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateUObject(this, &UPaperZDAnimSequenceFactory::OnTargetAnimBPSelected);

		/** The default view mode should be a list view */
		AssetPickerConfig.InitialAssetViewType = EAssetViewType::List;

		PickerWindow = SNew(SWindow)
			.Title(LOCTEXT("CreateAnimSequenceOptions", "Pick Parent Animation Blueprint"))
			.ClientSize(FVector2D(500, 600))
			.SupportsMinimize(false).SupportsMaximize(false)
			[
				SNew(SBorder)
				.BorderImage(FEditorStyle::GetBrush("Menu.Background"))
			[
				ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
			]
			];

		GEditor->EditorAddModalWindow(PickerWindow.ToSharedRef());
		PickerWindow.Reset();
	}

	//Will work if we have an AnimBP
	return TargetAnimBP != nullptr && AnimSequenceClass != nullptr;
}

UObject* UPaperZDAnimSequenceFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	//Use the selected class, or fallback to the given class (shouldn't happen)
	UClass* ClassToUse = AnimSequenceClass != nullptr ? AnimSequenceClass.Get() : Class;
	if (TargetAnimBP)
	{
		UPaperZDAnimSequence* AnimSequence = NewObject<UPaperZDAnimSequence>(InParent, ClassToUse, Name, Flags);
		AnimSequence->SetAnimBP(TargetAnimBP);
		return AnimSequence;
	}

	return NULL;
}

void UPaperZDAnimSequenceFactory::OnTargetAnimBPSelected(const FAssetData& SelectedAsset)
{
	TargetAnimBP = Cast<UPaperZDAnimBP>(SelectedAsset.GetAsset());
	PickerWindow->RequestDestroyWindow();
}

bool UPaperZDAnimSequenceFactory::PickAvailableClass(UClass*& PickedClass) const
{
	//Create a picker for the class that will inherit from
	FClassViewerModule&  ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");
	FClassViewerInitializationOptions InitOptions;
	InitOptions.Mode = EClassViewerMode::ClassPicker;

	//Initialize filter
	TSharedPtr<FPaperZDAnimSequenceFactoryClassFilter> Filter = MakeShareable<FPaperZDAnimSequenceFactoryClassFilter>(new FPaperZDAnimSequenceFactoryClassFilter);
	InitOptions.ClassFilter = Filter;

	//View config
	const FText TitleText = LOCTEXT("PickAnimSequenceParentClass", "Pick a parent class");
	return SClassPickerDialog::PickClass(TitleText, InitOptions, PickedClass, UPaperZDAnimSequence::StaticClass());
}

#undef LOCTEXT_NAMESPACE
