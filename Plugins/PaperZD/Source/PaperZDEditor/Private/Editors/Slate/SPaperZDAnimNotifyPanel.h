// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Framework/Commands/Commands.h"
#include "AssetData.h"
//For curve track
#include "SCurveEditor.h"

DECLARE_DELEGATE_RetVal(float, FOnGetPlaybackValue)
DECLARE_DELEGATE_RetVal(uint32, FOnGetNumKeys)
DECLARE_DELEGATE_RetVal(float, FOnGetSequenceLength)
DECLARE_DELEGATE_TwoParams(FOnSetViewRange, float, float)
DECLARE_DELEGATE_OneParam(FOnNotifySelectionChanged, TArray<UObject*>)

class SBorder;
class SScrollBar;
class FUICommandList;
class SPaperZDAnimNotifyTrack;
class UPaperZDAnimNotify_Base;

class FPaperZDAnimNotifyPanelCommands : public TCommands<FPaperZDAnimNotifyPanelCommands>
{
public:
	FPaperZDAnimNotifyPanelCommands();	

	TSharedPtr<FUICommandInfo> DeleteNotify;

	virtual void RegisterCommands() override;
};

class SPaperZDAnimNotifyPanel: public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPaperZDAnimNotifyPanel)
	{}

	SLATE_ARGUMENT(class UPaperZDAnimSequence*, AnimSequence)
	SLATE_ARGUMENT(class UPaperZDAnimBP*, AnimBP)
	SLATE_ARGUMENT(float, WidgetWidth)
	SLATE_ATTRIBUTE(float, ViewInputMin)
	SLATE_ATTRIBUTE(float, ViewInputMax)
	SLATE_EVENT(FOnGetPlaybackValue, OnGetPlaybackValue)
	SLATE_EVENT(FOnGetNumKeys, OnGetNumKeys)
	SLATE_EVENT(FOnGetSequenceLength, OnGetSequenceLength)
	SLATE_EVENT(FOnSetViewRange, OnSetViewRange)
	SLATE_EVENT(FOnNotifySelectionChanged, OnNotifySelectionChanged)
	SLATE_END_ARGS()

	~SPaperZDAnimNotifyPanel();

	void Construct(const FArguments& InArgs);
	void BindCommands();
	void RefreshNotifyTracks();
	void Update();
	void OnNotifyTrackScrolled(float InScrollOffsetFraction);
	void InputViewRangeChanged(float ViewMin, float ViewMax);
	FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent);
	FReply InsertTrack(int32 InsertInto);
	FReply DeleteTrack(int32 TrackIndex);
	bool CanDeleteTrack(int TrackIndex);
	FReply OnNotifyNodesDragStarted(const FVector2D& ScreenCursorPos,const bool bDragOnMarker);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime);
	//virtual void OnFocusLost(const FFocusEvent& InFocusEvent) override;
	
	//For getting the notify data
	void OnGetNativeNotifyData(TArray<UClass*>& OutClasses, UClass* NotifyOutermost, TArray<FString>* OutAllowedBlueprintClassNames);

	//Listener when a node is clicked and should change the selection
	void OnNodeClicked();

	//Get the blueprints that inherit from the anim notifies classes
	void OnGetNotifyBlueprintData(TArray<FAssetData>& OutNotifyData, TArray<FString>* InOutAllowedClassNames);
	void OnDeletePressed();
	void DeselectAllNotifies();
	void DeleteSelectedNodes();
	void UnregisterSelectedCustomNotify(class UPaperZDAnimNotifyCustom* CustomNotify);
	
	//For getting selections
	TArray<UPaperZDAnimNotify_Base*> GetAllSelectedNotifyObjects();

	template<class T>
	TArray<T*> GetAllSelectedNotifyObjects();

	//PostUndo Listener
	void OnPostUndo();

	/** We support keyboard focus to detect when we should process key commands like delete */
	virtual bool SupportsKeyboardFocus() const override
	{
		return true;
	}

private:
	TSharedPtr<SBorder> PanelArea;
	TSharedPtr<SScrollBar> NotifyTrackScrollBar;
	TSharedPtr<FUICommandList> UICommandList;
	TArray<TSharedPtr<class SPaperZDAnimNotifyTrack>> NotifyTracks;
	TArray<FString> NotifyClassNames;
	TArray<FString> NotifyStateClassNames;

	TWeakObjectPtr<UPaperZDAnimSequence> AnimSequence;

	//Handlers
	FOnGetPlaybackValue OnGetPlaybackValue;
	FOnGetNumKeys OnGetNumKeys;
	FOnGetSequenceLength OnGetSequenceLength;
	FOnSetViewRange OnSetViewRange;
	FOnNotifySelectionChanged OnNotifySelectionChanged;
	
	/** Controls the width of the tracks column */
	float WidgetWidth;

	TAttribute<float> ViewInputMin;
	TAttribute<float> ViewInputMax;
};