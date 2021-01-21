// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "PaperZDAnimationTrackEditor.h"
#include "PaperZDEditor.h"
#include "PaperZDAnimBP.h"
#include "Sequencer/PaperZDMovieSceneAnimationTrack.h"
#include "Sequencer/PaperZDMovieSceneAnimationSection.h"
#include "PaperZDCharacter.h"
#include "ISequencerTrackEditor.h"
#include "ISequencer.h"
#include "SequencerSectionPainter.h"
#include "Modules/ModuleManager.h"
#include "AssetRegistryModule.h"
#include "PaperFlipbook.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "CommonMovieSceneTools.h"
#include "AnimSequences/PaperZDAnimSequence.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Text/STextBlock.h"
#include "Editors/Slate/SPaperZDAnimSequenceList.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

//Style
#include "EditorStyleSet.h"


namespace PaperZDAnimationEditorConstants
{
	// @todo Sequencer Allow this to be customizable
	const uint32 AnimationTrackHeight = 20;
}

#define LOCTEXT_NAMESPACE "FPaperZDAnimationTrackEditor"

//////////////////////////////////////////////////////////////////////////
//// PaperZDAnimationSection
//////////////////////////////////////////////////////////////////////////

FPaperZDAnimationSection::FPaperZDAnimationSection(UMovieSceneSection& InSection, FPaperZDAnimationTrackEditor *InTrackEditor)
	: Section(*CastChecked<UPaperZDMovieSceneAnimationSection>(&InSection))
	, TrackEditor(InTrackEditor)
{
}

UMovieSceneSection* FPaperZDAnimationSection::GetSectionObject()
{
	return &Section;
}

FText FPaperZDAnimationSection::GetSectionTitle() const
{
	if (Section.Params.Animation->IsValidLowLevel())
	{
		return FText::FromName(Section.Params.Animation->GetSequenceName());
	}
	return LOCTEXT("InvalidAnimationSection", "Invalid Sequence");
}

float FPaperZDAnimationSection::GetSectionHeight() const
{
	return PaperZDAnimationEditorConstants::AnimationTrackHeight;
}

int32 FPaperZDAnimationSection::OnPaintSection(FSequencerSectionPainter& Painter) const
{
	const ESlateDrawEffect DrawEffects = Painter.bParentEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;

	const FTimeToPixel& TimeToPixelConverter = Painter.GetTimeConverter();
	static const FSlateBrush* GenericDivider = FEditorStyle::GetBrush("Sequencer.GenericDivider");

	int32 LayerId = Painter.PaintSectionBackground();

	// Add lines where the animation starts and ends/loops
	float AnimPlayRate = FMath::IsNearlyZero(Section.Params.PlayRate) ? 1.0f : Section.Params.PlayRate;
	float SeqLength = (Section.Params.GetSequenceLength() - (Section.Params.StartOffset + Section.Params.EndOffset)) / AnimPlayRate;

	FFrameRate TickResolution = TimeToPixelConverter.GetTickResolution();
	if (!FMath::IsNearlyZero(SeqLength, KINDA_SMALL_NUMBER) && SeqLength > 0)
	{
		float MaxOffset = Section.GetRange().Size<FFrameTime>() / TickResolution;
		float OffsetTime = SeqLength;
		float StartTime = Section.GetInclusiveStartFrame() / TickResolution;

		while (OffsetTime < MaxOffset)
		{
			float OffsetPixel = TimeToPixelConverter.SecondsToPixel(StartTime + OffsetTime) - TimeToPixelConverter.SecondsToPixel(StartTime);

			FSlateDrawElement::MakeBox(
				Painter.DrawElements,
				LayerId,
				Painter.SectionGeometry.MakeChild(
					FVector2D(2.f, Painter.SectionGeometry.Size.Y - 2.f),
					FSlateLayoutTransform(FVector2D(OffsetPixel, 1.f))
				).ToPaintGeometry(),
				GenericDivider,
				DrawEffects
			);

			OffsetTime += SeqLength;
		}
	}

	return LayerId;
}

void FPaperZDAnimationSection::BeginResizeSection()
{
	InitialStartOffsetDuringResize = Section.Params.StartOffset;
	InitialStartTimeDuringResize = Section.HasStartFrame() ? Section.GetInclusiveStartFrame() : 0;
}

void FPaperZDAnimationSection::ResizeSection(ESequencerSectionResizeMode ResizeMode, FFrameNumber ResizeTime)
{
	// Adjust the start offset when resizing from the beginning
	if (ResizeMode == SSRM_LeadingEdge)
	{
		FFrameRate FrameRate = Section.GetTypedOuter<UMovieScene>()->GetTickResolution();
		float StartOffset = (ResizeTime - InitialStartTimeDuringResize) * Section.Params.PlayRate / FrameRate;
		StartOffset += InitialStartOffsetDuringResize;

		// Ensure start offset is not less than 0 and adjust ResizeTime
		if (StartOffset < 0)
		{
			ResizeTime = ResizeTime - ((StartOffset / Section.Params.PlayRate) * FrameRate).RoundToFrame();

			StartOffset = 0.f;
		}

		Section.Params.StartOffset = StartOffset;
	}

	ISequencerSection::ResizeSection(ResizeMode, ResizeTime);
}

void FPaperZDAnimationSection::BuildSectionContextMenu(FMenuBuilder& MenuBuilder, const FGuid& ObjectBinding)
{
	MenuBuilder.AddSubMenu(
		LOCTEXT("ReplaceAnimation", "Replace Animation"), NSLOCTEXT("ZDSequencer", "ReplaceAnimationTooltip", "Replace an Animation Sequence with another."),
		FNewMenuDelegate::CreateRaw(TrackEditor, &FPaperZDAnimationTrackEditor::AddAnimationSubMenu, ObjectBinding, &Section)
	);
}

//////////////////////////////////////////////////////////////////////////
//// PaperZDAnimationTrackEditor
//////////////////////////////////////////////////////////////////////////
FPaperZDAnimationTrackEditor::FPaperZDAnimationTrackEditor(TSharedRef<ISequencer> OwningSequencer) 
	: FMovieSceneTrackEditor(OwningSequencer)
{
}

TSharedRef<ISequencerTrackEditor> FPaperZDAnimationTrackEditor::CreateTrackEditor(TSharedRef<ISequencer> OwningSequencer)
{
	return MakeShareable(new FPaperZDAnimationTrackEditor(OwningSequencer));
}

bool FPaperZDAnimationTrackEditor::SupportsType(TSubclassOf<UMovieSceneTrack> Type) const
{
	return Type == UPaperZDMovieSceneAnimationTrack::StaticClass();
}


TSharedRef<ISequencerSection> FPaperZDAnimationTrackEditor::MakeSectionInterface(UMovieSceneSection& SectionObject, UMovieSceneTrack& Track, FGuid ObjectBinding)
{
	check(SupportsType(SectionObject.GetOuter()->GetClass()));
	return MakeShareable(new FPaperZDAnimationSection(SectionObject, this));
}

void FPaperZDAnimationTrackEditor::BuildObjectBindingTrackMenu(FMenuBuilder& MenuBuilder, const TArray<FGuid>& ObjectBindings, const UClass* ObjectClass)
{
	if (ObjectClass->IsChildOf(APaperZDCharacter::StaticClass()))
	{
		const TSharedPtr<ISequencer> ParentSequencer = GetSequencer();

		// Load the asset registry module
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

		// Collect a full list of assets with the specified class
		TArray<FAssetData> AssetDataList;
		AssetRegistryModule.Get().GetAssetsByClass(UPaperFlipbook::StaticClass()->GetFName(), AssetDataList, true);

		if (AssetDataList.Num() && ObjectBindings.Num() == 1)
		{
			UMovieSceneTrack* Track = nullptr;
			MenuBuilder.AddSubMenu(
				LOCTEXT("AddAnimation", "Animation"), NSLOCTEXT("Sequencer", "AddAnimationTooltip", "Adds an animation track."),
				FNewMenuDelegate::CreateRaw(this, &FPaperZDAnimationTrackEditor::AddAnimationSubMenu, ObjectBindings[0], Track)
			);
		}
	}
}

void FPaperZDAnimationTrackEditor::AddAnimationSubMenu(FMenuBuilder& MenuBuilder, FGuid ObjectBinding, UMovieSceneTrack* Track)
{
	TSharedPtr<ISequencer> SequencerPtr = GetSequencer();
	APaperZDCharacter* Character = Cast<APaperZDCharacter>(SequencerPtr->FindSpawnedObjectOrTemplate(ObjectBinding));
	const UPaperZDAnimBP* AnimationBlueprint = Character->AnimationBlueprint;

	TSharedPtr<SBox> MenuEntry;
	//Sanitary Check, the character could not have a AnimBP assigned
	if (!AnimationBlueprint)
	{
		MenuEntry = SNew(SBox)
			.WidthOverride(300.0f)
			.HeightOverride(300.f)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Justification(ETextJustify::Center)
				.Text(FText::FromString(TEXT("No Animation Blueprint Found. \nPlease configure the character's AnimBP")))
			];
	}
	else
	{
		MenuEntry = SNew(SBox)
			.WidthOverride(300.0f)
			.HeightOverride(300.f)
			[
				SNew(SPaperZDAnimSequenceList)
				.OnGetAnimSequences(FOnGetAnimSequences::CreateUObject(AnimationBlueprint, &UPaperZDAnimBP::GetAnimSequences))
				.OnAnimSequenceSelected(FOnAnimSequenceSelected::CreateRaw(this, &FPaperZDAnimationTrackEditor::OnAnimationSequenceSelected, ObjectBinding, Track))
			];
	}

	MenuBuilder.AddWidget(MenuEntry.ToSharedRef(), FText::GetEmpty(), true);
}

void FPaperZDAnimationTrackEditor::AddAnimationSubMenu(FMenuBuilder& MenuBuilder, FGuid ObjectBinding, UPaperZDMovieSceneAnimationSection *Section)
{
	TSharedPtr<ISequencer> SequencerPtr = GetSequencer();
	APaperZDCharacter* Character = Cast<APaperZDCharacter>(SequencerPtr->FindSpawnedObjectOrTemplate(ObjectBinding));
	const UPaperZDAnimBP* AnimationBlueprint = Character->AnimationBlueprint;

	TSharedPtr<SBox> MenuEntry;
	//Sanitary Check, the character could not have a AnimBP assigned
	if (!AnimationBlueprint)
	{
		MenuEntry = SNew(SBox)
			.WidthOverride(300.0f)
			.HeightOverride(300.f)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Justification(ETextJustify::Center)
			.Text(FText::FromString(TEXT("No Animation Blueprint Found. \nPlease configure the character's AnimBP")))
			];
	}
	else
	{
		//If there's a AnimBP bound, we can create a AnimSequenceList with it
		MenuEntry = SNew(SBox)
			.WidthOverride(300.0f)
			.HeightOverride(300.f)
			[
				SNew(SPaperZDAnimSequenceList)
				.OnGetAnimSequences(FOnGetAnimSequences::CreateUObject(AnimationBlueprint, &UPaperZDAnimBP::GetAnimSequences))
				.OnAnimSequenceSelected(FOnAnimSequenceSelected::CreateRaw(this, &FPaperZDAnimationTrackEditor::OnAnimationSequenceSelected, ObjectBinding, Section))
			];
	}

	MenuBuilder.AddWidget(MenuEntry.ToSharedRef(), FText::GetEmpty(), true);
}

void FPaperZDAnimationTrackEditor::OnAnimationSequenceSelected(UPaperZDAnimSequence* SelectedSequence, ESelectInfo::Type Type, FGuid ObjectBinding, UMovieSceneTrack* Track)
{
	FSlateApplication::Get().DismissAllMenus();
	TSharedPtr<ISequencer> SequencerPtr = GetSequencer();

	if (SelectedSequence && SequencerPtr.IsValid())
	{
		UObject* Object = SequencerPtr->FindSpawnedObjectOrTemplate(ObjectBinding);
		int32 RowIndex = INDEX_NONE;
		AnimatablePropertyChanged(FOnKeyProperty::CreateRaw(this, &FPaperZDAnimationTrackEditor::AddKeyInternal, Object, SelectedSequence, Track, RowIndex));
	}
}

void FPaperZDAnimationTrackEditor::OnAnimationSequenceSelected(UPaperZDAnimSequence* SelectedSequence, ESelectInfo::Type Type, FGuid ObjectBinding, UPaperZDMovieSceneAnimationSection *Section)
{
	FSlateApplication::Get().DismissAllMenus();

	if (SelectedSequence)
	{
		Section->Params.Animation = SelectedSequence;
		Section->MarkAsChanged();
	}
}

FKeyPropertyResult FPaperZDAnimationTrackEditor::AddKeyInternal(FFrameNumber KeyTime, UObject* Object, UPaperZDAnimSequence* AnimSequence, UMovieSceneTrack* Track, int32 RowIndex)
{
	FKeyPropertyResult KeyPropertyResult;

	FFindOrCreateHandleResult HandleResult = FindOrCreateHandleToObject(Object);
	FGuid ObjectHandle = HandleResult.Handle;
	KeyPropertyResult.bHandleCreated |= HandleResult.bWasCreated;
	if (ObjectHandle.IsValid())
	{
		if (!Track)
		{
			Track = AddTrack(GetSequencer()->GetFocusedMovieSceneSequence()->GetMovieScene(), ObjectHandle, UPaperZDMovieSceneAnimationTrack::StaticClass(), NAME_None);
			KeyPropertyResult.bTrackCreated = true;
		}

		if (ensure(Track))
		{
			Cast<UPaperZDMovieSceneAnimationTrack>(Track)->AddNewAnimationOnRow(KeyTime, AnimSequence, RowIndex);
			KeyPropertyResult.bTrackModified = true;
		}
	}

	return KeyPropertyResult;
}

#undef LOCTEXT_NAMESPACE