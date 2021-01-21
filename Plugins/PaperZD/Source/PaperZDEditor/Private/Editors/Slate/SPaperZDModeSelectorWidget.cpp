// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "SPaperZDModeSelectorWidget.h"
#include "PaperZDEditor.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Editors/Util/PaperZDEditorStyle.h"
#include "EditorStyleSet.h"

#define LOCTEXT_NAMESPACE "PaperZD_ModeSelector"

class SPaperZDModeSelectorWidgetSection : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPaperZDModeSelectorWidgetSection)
	{
	}
	SLATE_EVENT(FOnModeSelected, OnModeSelected)
	SLATE_EVENT(FOnGetCurrentSectionIdentifier, OnGetCurrentSectionIdentifier)
	SLATE_END_ARGS()

private:
	FOnModeSelected OnModeSelected;
	FChangeModeInfo Info;
	FOnGetCurrentSectionIdentifier OnGetCurrentSectionIdentifier;

public:
	void Construct(const FArguments& InArgs, FChangeModeInfo ModeInfo)
	{
		Info = ModeInfo;
		OnModeSelected = InArgs._OnModeSelected;
		OnGetCurrentSectionIdentifier = InArgs._OnGetCurrentSectionIdentifier;

		ChildSlot
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			[
				SNew(SCheckBox)
				.Style(FEditorStyle::Get(), "ToolBar.ToggleButton")
				.ForegroundColor(FSlateColor::UseForeground())
				.Padding(0.0f)
				.OnCheckStateChanged(this, &SPaperZDModeSelectorWidgetSection::OnCheckboxStateChanged)
				.IsChecked(this, &SPaperZDModeSelectorWidgetSection::GetCheckboxState)
				.ToolTipText(this, &SPaperZDModeSelectorWidgetSection::GetTooltipText)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.AutoWidth()
					[
						SNew(SBorder)
						.Padding(4.0f)
						.BorderImage(FEditorStyle::GetBrush("PropertyEditor.AssetThumbnailShadow"))
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							[
								SNew(SImage)
								.Image(FPaperZDEditorStyle::Get().GetBrush(Info.BrushName))
							]
						]
					]

					+SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(5.0f)
					[
						SNew(STextBlock)
						.Text(this, &SPaperZDModeSelectorWidgetSection::GetDisplayName)
						.TextStyle(FEditorStyle::Get(), "Toolbar.Label")
						.ShadowOffset(FVector2D::UnitVector)
					]
				]
			]
		];
	}

	FText GetDisplayName() const
	{
		return FText::FromName(Info.DisplayName);
	}

	ECheckBoxState GetCheckboxState() const
	{
		//Check if the callback function exists
		if (!OnGetCurrentSectionIdentifier.IsBound())
			return ECheckBoxState::Undetermined;

		return OnGetCurrentSectionIdentifier.Execute().Equals(Info.Identifier) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	FText GetTooltipText() const
	{
		return Info.TooltipText;
	}

	void OnCheckboxStateChanged(ECheckBoxState InState)
	{
		//Call the delegate
		OnModeSelected.ExecuteIfBound(Info.Identifier);
	}

};

void SPaperZDModeSelectorWidget::Construct(const FArguments& InArgs, TArray<FChangeModeInfo> ChangeModeInfo)
{
	OnModeSelected = InArgs._OnModeSelected;

	//Create the horizontal box that will hold this
	TSharedRef<SHorizontalBox> HorizontalBox = SNew(SHorizontalBox);

	for (int i = 0; i < ChangeModeInfo.Num(); i++)
	{
		//Check if we should add a separator
		if (i > 0)
		{
			HorizontalBox->AddSlot()
			.AutoWidth()
			.Padding(5.0f, 0.0f)
			.VAlign(VAlign_Center)
				[
					SNew(SImage)
					.Image(FPaperZDEditorStyle::Get().GetBrush("PaperZDEditor.ModeSwitcher.Separator"))
				];
		}

		//We assume the first one is checked, could change this?
		CurrentSectionIdentifier = InArgs._OnGetCurrentSectionIdentifier.Execute();

		FChangeModeInfo Info = ChangeModeInfo[i];
		HorizontalBox->AddSlot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, i == ChangeModeInfo.Num() - 1 ? 0.0f : 2.0f, 0.0f)
			[
				SNew(SPaperZDModeSelectorWidgetSection, Info)
				.OnGetCurrentSectionIdentifier(InArgs._OnGetCurrentSectionIdentifier)
				.OnModeSelected(FOnModeSelected::CreateSP(this,&SPaperZDModeSelectorWidget::HandleSectionSelected))
			];			
	}

	//Configure the child slot
	ChildSlot
	[
		HorizontalBox
	];
}

void SPaperZDModeSelectorWidget::HandleSectionSelected(FString Identifier)
{
	//Store the current section so the children can be updated
	CurrentSectionIdentifier = Identifier;

	//Finally call the delegate
	OnModeSelected.ExecuteIfBound(Identifier);
}
#undef LOCTEXT_NAMESPACE