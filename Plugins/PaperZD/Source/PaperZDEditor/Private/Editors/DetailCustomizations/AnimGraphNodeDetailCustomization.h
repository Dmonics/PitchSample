// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperZDAnimGraphNode_State.h"
#include "Slate/SPaperZDAnimSequenceList.h"
#include "Editor/PropertyEditor/Public/IDetailCustomization.h"

/**
 * 
 */
class FAnimGraphNodeDetailCustomization : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	FText GetAnimSequenceDisplayName() const;
	TSharedRef<class SWidget> GetMenuContent() const;
	void HandleMenuOpenChanged(bool bOpen);
	TArray<class UPaperZDAnimSequence*> GetAnimSequences();
	void HandleAnimSequenceSelected(class UPaperZDAnimSequence *SelectedSequence, ESelectInfo::Type SelectionType);

protected:
	TSharedPtr<class SComboButton> SequenceComboButton;
	TWeakObjectPtr<class UPaperZDAnimBP> AnimBPBeingEdited;
	TSharedPtr<class IPropertyHandle> AnimSequenceIdentifierHandle;
};
