// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimGraphNodeDetailCustomization.h"
#include "Editor/PropertyEditor/Public/DetailLayoutBuilder.h"
#include "Editor/PropertyEditor/Public/DetailCategoryBuilder.h"
#include "Editor/PropertyEditor/Public/DetailWidgetRow.h"
#include "Editors/Slate/SPaperZDAnimSequenceList.h"
#include "PaperZDAnimBP.h"
#include "PaperZDAnimGraph.h"
#include "AnimSequences/PaperZDAnimSequence.h"
#include "Widgets/Input/SComboButton.h"
#include "SResetSequenceToDefault.h"

#define LOCTEXT_NAMESPACE "StateGraphNodeDetailCustomization"
TSharedRef<IDetailCustomization> FAnimGraphNodeDetailCustomization::MakeInstance()
{
	return MakeShareable(new FAnimGraphNodeDetailCustomization);
}

void FAnimGraphNodeDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> ObjectsCustomized;
	DetailBuilder.GetObjectsBeingCustomized(ObjectsCustomized);

	//Only support single selection
	if (ObjectsCustomized.Num() != 1)
	{
		return;
	}

	//Store for later use
	UPaperZDAnimGraphNode *CustomizedState = Cast<UPaperZDAnimGraphNode>(ObjectsCustomized[0].Get()); //Translating from UObject to State
	AnimSequenceIdentifierHandle = DetailBuilder.GetProperty("AnimSequence");
	
	//Abort if the AnimSequenceHandler is not found
	if (!AnimSequenceIdentifierHandle->IsValidHandle() || !CustomizedState)
	{
		return;
	}

	//We need to get the parent animbp so we can get the valid animsequences to use, we can traverse the Node->Graph->AnimBP pointer hierarchy
	AnimBPBeingEdited = CastChecked<UPaperZDAnimGraph>(CustomizedState->GetGraph())->GetAnimBP();

	IDetailCategoryBuilder& AnimationCategory = DetailBuilder.EditCategory("Animation");
	DetailBuilder.EditCategory("_Animation").SetCategoryVisibility(false); //As the PropertyHandle doesn't appear unless it's set as a Edit__ property, we need to hide the default widget
	
	//Create the combo button
	SequenceComboButton = SNew(SComboButton)
		.ToolTipText(LOCTEXT("AnimSequenceTooltip","The AnimSequence to use on this state"))
		.ButtonStyle( FEditorStyle::Get(), "PropertyEditor.AssetComboStyle" )
		.ForegroundColor(FEditorStyle::GetColor("PropertyEditor.AssetName.ColorAndOpacity"))
		.OnGetMenuContent( this, &FAnimGraphNodeDetailCustomization::GetMenuContent)
		.OnMenuOpenChanged( this, &FAnimGraphNodeDetailCustomization::HandleMenuOpenChanged )
		//.IsEnabled( IsEnabledAttribute )
		.ContentPadding(2.0f)
		.ButtonContent()
		[
			SNew(SHorizontalBox)
			/*+SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.Image( this, &SPropertyEditorAsset::GetStatusIcon )
			]*/

			+SHorizontalBox::Slot()
			.FillWidth(1)
			.VAlign(VAlign_Center)
			[
				// Show the name of the asset or actor
				SNew(STextBlock)
				.TextStyle( FEditorStyle::Get(), "PropertyEditor.AssetClass" )
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(this, &FAnimGraphNodeDetailCustomization::GetAnimSequenceDisplayName)
			]
		];

	AnimationCategory.AddCustomRow(LOCTEXT("AnimSequenceDisplayName", "Animation Sequence"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("AnimSequenceLabel", "AnimSequence"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent().MinDesiredWidth(300)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1)
			[
				SequenceComboButton.ToSharedRef()
			]
			+ SHorizontalBox::Slot()
			.Padding(4.0f, 0.0f)
			.AutoWidth()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Left)
			[
				SNew(SResetSequenceToDefault,AnimSequenceIdentifierHandle)
			]
		];
}

FText FAnimGraphNodeDetailCustomization::GetAnimSequenceDisplayName() const
{
	if (AnimSequenceIdentifierHandle->IsValidHandle())
	{
		UObject* OutValue = nullptr;
		AnimSequenceIdentifierHandle->GetValue(OutValue);

		if (OutValue)
		{
			UPaperZDAnimSequence* AnimSequence = CastChecked<UPaperZDAnimSequence>(OutValue);
			return FText::FromName(AnimSequence->GetSequenceName());
		}
		else
		{
			return FText::FromName(NAME_None);
		}
	}
	else
	{
		return FText::FromString("Invalid");
	}
}

TSharedRef<SWidget> FAnimGraphNodeDetailCustomization::GetMenuContent() const
{
	return SNew(SPaperZDAnimSequenceList)
		.OnGetAnimSequences(FOnGetAnimSequences::CreateSP(this, &FAnimGraphNodeDetailCustomization::GetAnimSequences))
		.OnAnimSequenceSelected(this, &FAnimGraphNodeDetailCustomization::HandleAnimSequenceSelected);
}

void FAnimGraphNodeDetailCustomization::HandleMenuOpenChanged(bool bOpen)
{
	if (!bOpen)
	{
		SequenceComboButton->SetMenuContent(SNullWidget::NullWidget);
	}
}


TArray<UPaperZDAnimSequence*> FAnimGraphNodeDetailCustomization::GetAnimSequences()
{
	return AnimBPBeingEdited.IsValid() ? AnimBPBeingEdited->GetAnimSequences() : TArray<UPaperZDAnimSequence*>();
}

void FAnimGraphNodeDetailCustomization::HandleAnimSequenceSelected(UPaperZDAnimSequence *SelectedSequence, ESelectInfo::Type SelectionType)
{
	if (AnimSequenceIdentifierHandle->IsValidHandle())
	{
		AnimSequenceIdentifierHandle->SetValue(SelectedSequence);
	}

	//Close the Menu
	SequenceComboButton->SetIsOpen(false);
}
#undef LOCTEXT_NAMESPACE
