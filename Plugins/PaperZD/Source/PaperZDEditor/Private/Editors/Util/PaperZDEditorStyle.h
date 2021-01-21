// Copyright 2017-2018 Critical Failure Studio

#pragma once

#include "Styling/ISlateStyle.h"

class FPaperZDEditorStyle
{
public:

	static void Initialize();

	static void Shutdown();

	static const ISlateStyle& Get();

	static FName GetStyleSetName();	

private:

	static TSharedRef< class FSlateStyleSet > Create();

private:

	static TSharedPtr< class FSlateStyleSet > PaperZDEditorStyleInstance;
};
