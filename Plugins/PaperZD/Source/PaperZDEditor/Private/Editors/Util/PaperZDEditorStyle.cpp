// Copyright 2017-2018 Critical Failure Studio

#include "PaperZDEditorStyle.h"

#include "Framework/Application/SlateApplication.h"
#include "EditorStyleSet.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"

TSharedPtr<FSlateStyleSet> FPaperZDEditorStyle::PaperZDEditorStyleInstance = nullptr;

void FPaperZDEditorStyle::Initialize()
{
	if (!PaperZDEditorStyleInstance.IsValid())
	{
		PaperZDEditorStyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*PaperZDEditorStyleInstance);
	}
}

void FPaperZDEditorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*PaperZDEditorStyleInstance);
	ensure(PaperZDEditorStyleInstance.IsUnique());
	PaperZDEditorStyleInstance.Reset();
}

FName FPaperZDEditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("PaperZDEditorStyle"));
	return StyleSetName;
}

const ISlateStyle& FPaperZDEditorStyle::Get()
{
	return *PaperZDEditorStyleInstance;
}


#define IMAGE_BRUSH(RelativePath, ...)	FSlateImageBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BOX_BRUSH(RelativePath, ...)	FSlateBoxBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BORDER_BRUSH(RelativePath, ...) FSlateBorderBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define TTF_FONT(RelativePath, ...)		FSlateFontInfo(Style->RootToContentDir(RelativePath, TEXT(".ttf")), __VA_ARGS__)
#define OTF_FONT(RelativePath, ...)		FSlateFontInfo(Style->RootToContentDir(RelativePath, TEXT(".otf")), __VA_ARGS__)

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon16x24(16.0f, 24.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon25x25(25.0f, 25.0f);
const FVector2D Icon30x30(30.0f, 30.0f);
const FVector2D Icon40x40(40.0f, 40.0f);
const FVector2D Icon40x25(40.0f, 25.0f);
const FVector2D Icon50x50(50.0f, 50.0f);
const FVector2D Icon60x60(60.0f, 60.0f);
const FVector2D Icon120x120(120.0f, 120.0f);

TSharedRef<FSlateStyleSet> FPaperZDEditorStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	FString ContentDir = IPluginManager::Get().FindPlugin("PaperZD")->GetContentDir();
	Style->SetContentRoot(ContentDir);

	//Switcher
	Style->Set("PaperZDEditor.ModeSwitcher.Blueprint", new IMAGE_BRUSH("Icons/Switcher/BlueprintMode", Icon50x50));
	Style->Set("PaperZDEditor.ModeSwitcher.Animation", new IMAGE_BRUSH("Icons/Switcher/AnimationMode", Icon50x50));
	Style->Set("PaperZDEditor.ModeSwitcher.Separator", new IMAGE_BRUSH("Icons/Switcher/PipelineSeparator", Icon16x24));

	//Sequences
	Style->Set("PaperZDEditor.Sequences.Avatar", new IMAGE_BRUSH("Icons/Graph/Sequence", Icon40x40));

	//AnimBP icon
	Style->Set("ClassThumbnail.PaperZDAnimBP", new IMAGE_BRUSH("Icons/AnimBP_Thumbnail", Icon120x120));
	Style->Set("ClassThumbnail.PaperZDAnimInstance", new IMAGE_BRUSH("Icons/AnimBP_Thumbnail", Icon120x120));

	//Graph Related
	Style->Set("PaperZDEditor.Graph.TransitionNode.Icon", new IMAGE_BRUSH("Icons/Graph/Trans_Arrow", Icon25x25));
	Style->Set("PaperZDEditor.Graph.TransitionNode.Body", new IMAGE_BRUSH("Icons/Graph/Trans_Body", Icon30x30));
	Style->Set("PaperZDEditor.Graph.TransitionNode.Body_Seq", new IMAGE_BRUSH("Icons/Graph/Trans_Seq", Icon40x25));

	return Style;
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT

#undef PARENT_DIR
