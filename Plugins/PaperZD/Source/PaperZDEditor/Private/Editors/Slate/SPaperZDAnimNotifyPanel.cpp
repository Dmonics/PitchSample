// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "SPaperZDAnimNotifyPanel.h"
#include "PaperZDEditor.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "SScrubWidget.h"
#include "SScrubControlPanel.h"
#include "SPaperZDConfirmDialog.h"
#include "ScopedTransaction.h"
#include "AssetRegistryModule.h"
#include "Fonts/FontMeasure.h"
#include "EditorStyleSet.h"
#include "Editor.h"


#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Commands/UICommandList.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Input/STextEntryPopup.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SScrollBar.h"

#include "Animation/AnimTypes.h"
#include "AnimNodes/PaperZDAnimNode_State.h"
#include "AnimSequences/PaperZDAnimSequence.h"
#include "PaperZDAnimGraphNode_State.h"
#include "Notifies/PaperZDAnimNotifyState.h"
#include "Notifies/PaperZDAnimNotify.h"
#include "Notifies/PaperZDAnimNotifyCustom.h"
#include "PaperZDAnimBP.h"
#include "Notifies/PaperZDAnimNotify_Base.h"

#include "Kismet2/BlueprintEditorUtils.h"

#define LOCTEXT_NAMESPACE "PaperZDAnimNotifyPanel"

// Track Panel drawing
const float NotificationTrackHeight = 20.0f;

// AnimNotify Drawing
const float NotifyHeightOffset = 0.f;
const float NotifyHeight = NotificationTrackHeight + 3.f;
const FVector2D ScrubHandleSize(8.f, NotifyHeight);
const FVector2D AlignmentMarkerSize(8.f, NotifyHeight);
const FVector2D TextBorderSize(1.f, 1.f);
const float ScrubHandleHitBuffer = 3.0f;


class SPaperZDAnimNotifyNode;

DECLARE_DELEGATE_RetVal_FourParams(FReply, FOnNotifyNodeDragStarted, TSharedRef<SPaperZDAnimNotifyNode>, const FPointerEvent&, const FVector2D&, const bool)
DECLARE_DELEGATE_RetVal_TwoParams(FReply, FOnNotifyNodesDragStarted,const FVector2D&, const bool)
DECLARE_DELEGATE(FOnDropOperationComplete)
DECLARE_DELEGATE(FOnUpdatePanel)
DECLARE_DELEGATE_OneParam(FOnGetBlueprintNotifyData, TArray<FAssetData>&)
DECLARE_DELEGATE_OneParam(FOnGetNativeNotifyClasses, TArray<UClass*>&)
DECLARE_DELEGATE(FOnDeselectAllNotifies)
DECLARE_DELEGATE_RetVal(TArray<UPaperZDAnimNotify_Base*>, FOnGetAllSelectedNotifyObjects)
DECLARE_DELEGATE(FOnDeleteSelectedNotifies)
DECLARE_DELEGATE_OneParam(FOnUnregisterCustomNotify, UPaperZDAnimNotifyCustom*)
DECLARE_DELEGATE(FOnNodeClicked)

enum EHitPosition {
	EHP_Hit = 0,
	EHP_Above = 1,
	EHP_Below = 2,
	EHP_Right = 4,
	EHP_Left = 8
};

namespace ENotifyStateHandleHit
{
	enum Type
	{
		Start,
		End,
		None
	};
}

//////////////////////////////////////////////////////////////////////////
// SPaperZDAnimNotifyNode Declaration
class SPaperZDAnimNotifyNode : public SLeafWidget
{
	SLATE_BEGIN_ARGS(SPaperZDAnimNotifyNode) :
		_ViewInputMin(),
		_ViewInputMax(),
		_OnNodeDragStarted()
	{}
	SLATE_ATTRIBUTE(float, ViewInputMin)
	SLATE_ATTRIBUTE(float, ViewInputMax)
	SLATE_ARGUMENT(UPaperZDAnimNotify_Base *, Notify)
	SLATE_EVENT(FOnNotifyNodeDragStarted, OnNodeDragStarted)
	SLATE_EVENT(FOnDeselectAllNotifies, OnDeselectAllNotifies)
	SLATE_EVENT(FOnNodeClicked, OnNodeClicked)
	SLATE_ARGUMENT(int, TrackIndex)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	// SWidget interface
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnFocusLost(const FFocusEvent& InFocusEvent) override;
	virtual bool SupportsKeyboardFocus() const override;
	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	// End of SWidget interface

	FVector2D GetSize() const;
	FVector2D GetPosition() const { return Position; }

	void UpdateGeometry(const FGeometry& AllottedGeometry);

	void DrawScrubHandle(float ScrubHandleCentre, FSlateWindowElementList& OutDrawElements, int32 ScrubHandleID, const FGeometry &AllottedGeometry, const FSlateRect& MyClippingRect, FLinearColor NodeColour) const;

	/** Returns the size of this notifies duration in screen space */
	float GetDurationSize() const { return NotifyDurationSizeX; }

	//Whether the point is on top of this track
	bool HitTest(FVector2D TestPoint, int32 &HitInfo);

	// Extra hit testing to decide whether or not the duration handles were hit on a state node
	ENotifyStateHandleHit::Type DurationHandleHitTest(const FVector2D& CursorScreenPosition) const;

	const FVector2D& GetScreenPosition() const
	{
		return ScreenPosition;
	}

	/** Cached owning track geometry */
	FGeometry CachedTrackGeometry;

	float GetWidgetPaddingLeft() {
		return GetPosition().X;
	}

	bool IsBeingDragged() { return bBeingDragged; }

public:
	/* The notify that we're representating */
	TWeakObjectPtr<UPaperZDAnimNotify_Base> Notify;
	bool bSelected;

private:
	FVector2D CachedAllotedGeometrySize;
	FVector2D Position;
	FVector2D Size;
	float NotifyTimePositionX;
	float NotifyDurationSizeX;
	float NotifyScrubHandleCentre;
	FVector2D ScreenPosition;
	FVector2D MouseDownLastPosition;
	int TrackIndex;

	//Handle used during duration modification
	ENotifyStateHandleHit::Type CurrentDragHandle;
	int32 DragMarkerTransactionIdx;

	TAttribute<float> ViewInputMin;
	TAttribute<float> ViewInputMax;

	bool bBeingDragged;
	bool bDrawTooltipToRight;

	TSharedPtr<SOverlay> EndMarkerNodeOverlay;

	FSlateFontInfo Font;
	FVector2D TextSize;
	float LabelWidth;

	//Events
	FOnNotifyNodeDragStarted OnNodeDragStarted;
	FOnDeselectAllNotifies OnDeselectAllNotifies;
	FOnNodeClicked OnNodeClicked;
};

//////////////////////////////////////////////////////////////////////////
// SPaperZDAnimNotifyTrack Declaration

class SPaperZDAnimNotifyTrack : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPaperZDAnimNotifyTrack)
	{}
	SLATE_ARGUMENT(UPaperZDAnimSequence *, AnimSequence)
	SLATE_ARGUMENT(int, TrackIndex)
	SLATE_ATTRIBUTE(float, ViewInputMin)
	SLATE_ATTRIBUTE(float, ViewInputMax)
	SLATE_EVENT(FOnGetPlaybackValue, OnGetPlaybackValue)
	SLATE_EVENT(FOnGetNumKeys, OnGetNumKeys)
	SLATE_EVENT(FOnGetSequenceLength, OnGetSequenceLength)
	SLATE_EVENT(FOnNotifyNodesDragStarted, OnNodesDragStarted)
	SLATE_EVENT(FOnUpdatePanel, OnUpdatePanel)
	SLATE_EVENT(FOnGetBlueprintNotifyData, OnGetNotifyBlueprintData)
	SLATE_EVENT(FOnGetBlueprintNotifyData, OnGetNotifyStateBlueprintData)
	SLATE_EVENT(FOnGetNativeNotifyClasses, OnGetNotifyNativeClasses)
	SLATE_EVENT(FOnGetNativeNotifyClasses, OnGetNotifyStateNativeClasses)
	SLATE_EVENT(FOnDeselectAllNotifies, OnDeselectAllNotifies)
	SLATE_EVENT(FOnGetAllSelectedNotifyObjects, OnGetAllSelectedNotifyObjects)
	SLATE_EVENT(FOnDeleteSelectedNotifies, OnDeleteSelectedNotifies)
	SLATE_EVENT(FOnUnregisterCustomNotify, OnUnregisterCustomNotify)
	SLATE_EVENT(FOnNodeClicked, OnNodeClicked)
	SLATE_END_ARGS()

	/** Type used for list widget of tracks */
	void Construct(const FArguments& InArgs);

	// SWidget interface
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override { UpdateCachedGeometry(AllottedGeometry); }
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	FReply OnNotifyNodeDragStarted(TSharedRef<SPaperZDAnimNotifyNode> NotifyNode, const FPointerEvent& MouseEvent, const FVector2D& ScreenNodePosition, const bool bDragOnMarker, int32 NotifyIndex);
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	// End of SWidget interface

	//Wether the point is on top of this track
	bool HitTest(FVector2D TestPoint, int32 &HitInfo);

	//Methods for context menu (nodes)
	void DeleteSelectedNotifies();
	void UnregisterCustomNotify(UPaperZDAnimNotifyCustom *Notify);

	bool HasInvalidNodes();

	/**
	* Update the nodes to match the data that the panel is observing
	*/
	void Update();

	//Disconnect The nodes and add it to the Drag Nodes Array
	void DisconnectSelectedNodesForDrag(TArray<TSharedPtr<SPaperZDAnimNotifyNode>>& DragNodes);

	//Deselect all nodes of this track
	void DeselectAllNodes();

	//Handle the drop of a node on this track, at that absolute position
	void HandleNodeDrop(TSharedPtr<SPaperZDAnimNotifyNode> Node, FVector2D AbsolutePosition);

	//Create the context menu
	TSharedPtr<SWidget> GetContextMenu(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent, const bool bHitNode);

	// Returns the padding needed to render the notify in the correct track position
	FMargin GetNotifyTrackPadding(int32 NotifyIndex) const
	{
		float LeftMargin = NotifyNodes[NotifyIndex]->GetWidgetPaddingLeft();
		float RightMargin = CachedGeometry.Size.X - NotifyNodes[NotifyIndex]->GetPosition().X - NotifyNodes[NotifyIndex]->GetSize().X;
		return FMargin(LeftMargin, 0, RightMargin, 0);
	}

	FVector2D ComputeDesiredSize(float) const override
	{
		FVector2D Size;
		Size.X = 200.0f;
		Size.Y = NotifyHeight;

		return Size;
	}

	void UpdateCachedGeometry(const FGeometry& InGeometry)
	{
		//@TODO: complete here
		CachedGeometry = InGeometry;

		for (TSharedPtr<SPaperZDAnimNotifyNode> Node : NotifyNodes)
		{
			Node->CachedTrackGeometry = InGeometry;
		}
	}

	FVector2D GetScreenPosition()
	{
		return CachedGeometry.AbsolutePosition;
	}

	//Get the time on the position
	float CalculateTime(const FGeometry& MyGeometry, FVector2D Position, bool bInputIsAbsolute = true);

	TArray<UPaperZDAnimNotify_Base *> GetAllSelectedNotifyObjects()
	{
		TArray<UPaperZDAnimNotify_Base*> SelectedNotifies;
		for (TSharedPtr<SPaperZDAnimNotifyNode> Node : NotifyNodes)
		{
			if (Node->bSelected)
				SelectedNotifies.Add(Node->Notify.Get());
		}

		return SelectedNotifies;
	}

	template <class T>
	TArray<T*> GetAllSelectedNotifyObjects()
	{
		TArray<T*> SelectedNotifies;
		for (TSharedPtr<SPaperZDAnimNotifyNode> Node : NotifyNodes)
		{
			if (Node->bSelected)
				SelectedNotifies.Add(Cast<T>(Node->Notify));
		}

		return SelectedNotifies;
	}

protected:

	// Data structure for blueprint notify context menu entries
	struct BlueprintNotifyMenuInfo
	{
		FName NotifyName;
		FString BlueprintPath;
		UClass* BaseClass;
	};

	//Commands
	void FillNewNotifyMenu(FMenuBuilder& MenuBuilderbool, bool bIsReplaceWithMenu = false);
	void FillNewNotifyStateMenu(FMenuBuilder& MenuBuilder, bool bIsReplaceWithMenu = false);

	// Format notify asset data into the information needed for menu display
	void GetNotifyMenuData(TArray<FAssetData>& NotifyAssetData, TArray<BlueprintNotifyMenuInfo>& OutNotifyMenuData);
	
	//New Notify Related
	void CreateNewNotifyAtCursor(FName NewNotifyName, UClass* NotifyClass);
	bool IsValidToPlace(UClass* NotifyClass) const;
	void CreateNewBlueprintNotifyAtCursor(FName NewNotifyName, FString BlueprintPath);
	void OnNewNotifyClicked();
	void AddNewNotify(const FText& NewNotifyName, ETextCommit::Type CommitInfo);

	//For getting the blueprint class name from path
	TSubclassOf<UObject> GetBlueprintClassFromPath(FString BlueprintPath)
	{
		TSubclassOf<UObject> BlueprintClass = NULL;
		if (!BlueprintPath.IsEmpty())
		{
			UBlueprint* BlueprintLibPtr = LoadObject<UBlueprint>(NULL, *BlueprintPath, NULL, 0, NULL);
			BlueprintClass = Cast<UClass>(BlueprintLibPtr->GeneratedClass);
		}
		return BlueprintClass;
	}

private:
	/* AnimSequence we're currently editing */
	TWeakObjectPtr<UPaperZDAnimSequence> AnimSequence;

	int32 TrackIndex;
	float LastClickedTime;

	TArray<TSharedPtr<SPaperZDAnimNotifyNode>> NotifyNodes;

	TSharedPtr<SBorder> TrackArea;
	TSharedPtr<SOverlay> NodeOverlay;

	TAttribute<float> ViewInputMin;
	TAttribute<float> ViewInputMax;

	//Handlers
	FOnGetPlaybackValue OnGetPlaybackValue;
	FOnGetNumKeys OnGetNumKeys;
	FOnGetSequenceLength OnGetSequenceLength;
	FOnNotifyNodesDragStarted OnNodesDragStarted;
	FOnUpdatePanel OnUpdatePanel;
	FOnGetBlueprintNotifyData OnGetNotifyBlueprintData;
	FOnGetBlueprintNotifyData OnGetNotifyStateBlueprintData;
	FOnGetNativeNotifyClasses OnGetNotifyNativeClasses;
	FOnGetNativeNotifyClasses OnGetNotifyStateNativeClasses;
	FOnDeselectAllNotifies OnDeselectAllNotifies;
	FOnGetAllSelectedNotifyObjects OnGetAllSelectedNotifyObjects;
	FOnDeleteSelectedNotifies OnDeleteSelectedNotifies;
	FOnUnregisterCustomNotify OnUnregisterCustomNotify;
	FOnNodeClicked OnNodeClicked;

	FGeometry CachedGeometry;
};

//////////////////////////////////////////////////////////////////////////
// SPaperZDEdTrack Declaration

class SPaperZDEdTrack : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPaperZDEdTrack)
	{}
	SLATE_ARGUMENT(float, WidgetWidth)
	SLATE_ARGUMENT(UPaperZDAnimSequence *, AnimSequence)
	SLATE_ARGUMENT(int32, TrackIndex)
	SLATE_ATTRIBUTE(float, ViewInputMin)
	SLATE_ATTRIBUTE(float, ViewInputMax)
	SLATE_ARGUMENT(TSharedPtr<SPaperZDAnimNotifyPanel>, AnimNotifyPanel)
	SLATE_EVENT(FOnGetPlaybackValue, OnGetPlaybackValue)
	SLATE_EVENT(FOnGetNumKeys, OnGetNumKeys)
	SLATE_EVENT(FOnGetSequenceLength, OnGetSequenceLength)
	SLATE_EVENT(FOnNotifyNodesDragStarted, OnNodesDragStarted)
	SLATE_EVENT(FOnUpdatePanel, OnUpdatePanel)
	SLATE_EVENT(FOnGetBlueprintNotifyData, OnGetNotifyBlueprintData)
	SLATE_EVENT(FOnGetBlueprintNotifyData, OnGetNotifyStateBlueprintData)
	SLATE_EVENT(FOnGetNativeNotifyClasses, OnGetNotifyNativeClasses)
	SLATE_EVENT(FOnGetNativeNotifyClasses, OnGetNotifyStateNativeClasses)
	SLATE_EVENT(FOnDeselectAllNotifies, OnDeselectAllNotifies)
	SLATE_EVENT(FOnGetAllSelectedNotifyObjects, OnGetAllSelectedNotifyObjects)
	SLATE_EVENT(FOnDeleteSelectedNotifies, OnDeleteSelectedNotifies)
	SLATE_EVENT(FOnUnregisterCustomNotify, OnUnregisterCustomNotify)
	SLATE_EVENT(FOnNodeClicked, OnNodeClicked)
	SLATE_END_ARGS()

/** Type used for list widget of tracks */
void Construct(const FArguments& InArgs);

//The notify track created
TSharedPtr<SPaperZDAnimNotifyTrack> NotifyTrack;

private:
	TSharedPtr<SPaperZDAnimNotifyPanel> PanelPtr;
};

//////////////////////////////////////////////////////////////////////////
// FNotifyDragDrop
class FNotifyDragDropOperation : public FDragDropOperation
{

private:
	// The nodes that are in the current selection
	TArray<TSharedPtr<SPaperZDAnimNotifyNode>> SelectedNodes;

	//The decorator to display
	TSharedPtr<SWidget> Decorator;

	//Method to call to update the view
	FOnDropOperationComplete OnDropComplete;

	//Offset between the mouse pointer and the beginning of the decorator
	FVector2D CursorOffset;

	//Size of the decorator
	FVector2D OverlayExtents;

	//The tracks
	TArray<TSharedPtr<SPaperZDAnimNotifyTrack>> NotifyTracks;

	//Last track hit by the cursor on test
	int LastHitTrackByCursor;

	//Info of the offsets
	TArray<FVector2D> OffsetInfo;

public:
	FNotifyDragDropOperation()
	{}

	~FNotifyDragDropOperation()
	{
		OnDropComplete.ExecuteIfBound();
	}

	//Static creator, to simplify the TSharedRef instantiation
	static TSharedRef<FNotifyDragDropOperation> Create(TArray<TSharedPtr<SPaperZDAnimNotifyNode>> InSelectedNodes, TSharedPtr<SWidget> InDecorator, FOnDropOperationComplete InOnDropComplete, FVector2D InCursorOffset, FVector2D InOverlayExtents, TArray<TSharedPtr<SPaperZDAnimNotifyTrack>> InTracks, TArray<FVector2D> InOffsetInfo)
	{
		TSharedRef<FNotifyDragDropOperation> Operation = MakeShareable(new FNotifyDragDropOperation());
		Operation->SelectedNodes = InSelectedNodes;
		Operation->Decorator = InDecorator;
		Operation->OnDropComplete = InOnDropComplete;
		Operation->CursorOffset = InCursorOffset;
		Operation->NotifyTracks = InTracks;
		Operation->LastHitTrackByCursor = 0;
		Operation->OverlayExtents = InOverlayExtents;
		Operation->OffsetInfo = InOffsetInfo;

		//Call construct, so the Cursor Window gets instantiated
		Operation->Construct();
		Operation->CursorDecoratorWindow->SetOpacity(0.5f);

		return Operation;
	}

	//Begin FDragDropOperation Interface
	virtual void OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) override
	{
		if (!bDropWasHandled)
		{
			//Iterate and give each track the nodes that corresponds
			for (int i = 0; i < SelectedNodes.Num(); i++)
			{
				TSharedPtr<SPaperZDAnimNotifyNode> Node = SelectedNodes[i];
				for (int j = 0; j < NotifyTracks.Num(); j++)
				{
					TSharedPtr<SPaperZDAnimNotifyTrack> Track = NotifyTracks[j];
					int32 HitInfo; 
					FVector2D NodeAbsolutePosition = CursorDecoratorWindow->GetClientRectInScreen().GetTopLeft() + OffsetInfo[i]; //@TODO: the decorator window has a padding
					bool Hit = Track->HitTest(NodeAbsolutePosition, HitInfo);

					//If a hit was made, or is a vertical Hit (outside the horizontal hitbox)
					if (Hit || (!(HitInfo & EHP_Above) && !(HitInfo & EHP_Below))) 
					{
						Track->HandleNodeDrop(Node, NodeAbsolutePosition);
						break;
					}
				}
			}

			OnDropComplete.ExecuteIfBound();
		}

		FDragDropOperation::OnDrop(bDropWasHandled, MouseEvent);
	}

	virtual void OnDragged(const class FDragDropEvent& DragDropEvent) override
	{
		//SelectedNodes[0]->TranslateWithCursorDelta(DragDropEvent.GetCursorDelta());
		FVector2D CursorPosition = DragDropEvent.GetScreenSpacePosition() + DragDropEvent.GetCursorDelta();

		//Move the window
		CursorDecoratorWindow->MoveWindowTo(GetLimitedOverlayOrigin(CursorPosition));
	}

	virtual TSharedPtr<SWidget> GetDefaultDecorator() const override
	{
		return Decorator;
	}
	//End FDragDropOperation Interface

private:
	//Gets the overlay origin, limited by the existing tracks
	FVector2D GetLimitedOverlayOrigin(FVector2D CursorPosition)
	{
		//Gets the offset on track numbers, between the cursor and the decorator window vertical origin
		int VerticalTrackOffset = CursorOffset.Y / NotifyHeight; //Always <= 0.
		int TrackExtentOffset = OverlayExtents.Y / NotifyHeight;

		//Hit test on tracks
		int MouseTrack = INDEX_NONE;
		for (int i = 0; i < NotifyTracks.Num(); i++) //The tracks are ordered top to bottom
		{
			TSharedPtr<SPaperZDAnimNotifyTrack> Track = NotifyTracks[i];
			int32 HitInfo;
			bool Hit = Track->HitTest(CursorPosition, HitInfo);

			if (Hit)
			{
				MouseTrack = i;
				break;
			}
			else if (i == 0 && (HitInfo & EHP_Above))
			{
				//The mouse is above the top track
				MouseTrack = 0;
				break;
			}
			else if (i == NotifyTracks.Num() - 1 && (HitInfo & EHP_Below))
			{
				//The mouse is below the bottom track, no sense on checking anymore
				MouseTrack = i;
				break;
			}
			else if (!(HitInfo & EHP_Above) && !( HitInfo & EHP_Below ))
			{
				//If the mouse hit on vertical, but not on horizontal, save the track
				MouseTrack = i;
				break;
			}
		}

		if (MouseTrack == INDEX_NONE)
		{
			//We haven't found a hit, most probably we are in between tracks (on the margins), fallback to last known track
			MouseTrack = LastHitTrackByCursor;
		}
		else
		{
			LastHitTrackByCursor = MouseTrack;
		}
		
		//Create the overlay origin
		FVector2D OverlayOrigin;
		
		//Get the valid tracks on each side (top and bottom)
		int ValidFirstTrack = FMath::Max(0, MouseTrack + VerticalTrackOffset);
		
		if (ValidFirstTrack + TrackExtentOffset > NotifyTracks.Num())
			ValidFirstTrack = NotifyTracks.Num() - TrackExtentOffset;

		//Just make sure we are not outside the scope
		ValidFirstTrack = FMath::Min(ValidFirstTrack, NotifyTracks.Num());

		//Store the Y Dimmension
		OverlayOrigin.Y = NotifyTracks[ValidFirstTrack]->GetScreenPosition().Y;

		//Compute X dimension
		OverlayOrigin.X = CursorPosition.X + CursorOffset.X;

		//@TODO: Create better X Dimension Limits

		return OverlayOrigin;
	}
};

//////////////////////////////////////////////////////////////////////////
// SPaperZDAnimNotifyNode Implementation
void SPaperZDAnimNotifyNode::Construct(const FArguments& InArgs)
{
	ViewInputMin = InArgs._ViewInputMin;
	ViewInputMax = InArgs._ViewInputMax;
	TrackIndex = InArgs._TrackIndex;

	Font = FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 10);

	Notify = InArgs._Notify;
	DragMarkerTransactionIdx = INDEX_NONE;
	CurrentDragHandle = ENotifyStateHandleHit::None;

	OnNodeDragStarted = InArgs._OnNodeDragStarted;
	OnDeselectAllNotifies = InArgs._OnDeselectAllNotifies;
	OnNodeClicked = InArgs._OnNodeClicked;

	if (const UPaperZDAnimNotifyState* StateNotify = Cast<const UPaperZDAnimNotifyState>(Notify))
	{
		SAssignNew(EndMarkerNodeOverlay, SOverlay);
	}
}

//Wether the point is on top of this track
bool SPaperZDAnimNotifyNode::HitTest(FVector2D TestPoint, int32 &HitInfo)
{
	//Init the variable
	HitInfo = EHP_Hit;

	FVector2D LowerRightPoint = ScreenPosition + Size;

	//If hit
	if (TestPoint >= ScreenPosition && TestPoint <= LowerRightPoint)
	{
		return true;
	}

	//No hit ocurred, check horizontal limits first
	if (TestPoint.X < ScreenPosition.X)
		HitInfo |= EHP_Left;
	else if (TestPoint.X > LowerRightPoint.X)
		HitInfo |= EHP_Right;

	//Check Vertical
	if (TestPoint.Y > ScreenPosition.Y)
		HitInfo |= EHP_Below;
	else if (TestPoint.Y < LowerRightPoint.Y)
		HitInfo |= EHP_Above;

	return false;
}

ENotifyStateHandleHit::Type SPaperZDAnimNotifyNode::DurationHandleHitTest(const FVector2D& CursorScreenPosition) const
{
	ENotifyStateHandleHit::Type MarkerHit = ENotifyStateHandleHit::None;

	// Make sure this node has a duration box (meaning it is a state node)
	if (NotifyDurationSizeX > 0.0f)
	{

		// Test for mouse inside duration box with handles included
		float ScrubHandleHalfWidth = ScrubHandleSize.X / 2.0f;

		// Position and size of the notify node including the scrub handles
		FVector2D NotifyNodePosition(NotifyScrubHandleCentre - ScrubHandleHalfWidth, 0.0f);
		FVector2D NotifyNodeSize(NotifyDurationSizeX + ScrubHandleHalfWidth * 2.0f, NotifyHeight);

		FVector2D MouseRelativePosition(GetCachedGeometry().AbsoluteToLocal(CursorScreenPosition));

		if (MouseRelativePosition > NotifyNodePosition && MouseRelativePosition < (NotifyNodePosition + NotifyNodeSize))
		{
			// Definitely inside the duration box, need to see which handle we hit if any
			if (MouseRelativePosition.X <= (NotifyNodePosition.X + ScrubHandleSize.X))
			{
				// Left Handle
				MarkerHit = ENotifyStateHandleHit::Start;
			}
			else if (MouseRelativePosition.X >= (NotifyNodePosition.X + NotifyNodeSize.X - ScrubHandleSize.X))
			{
				// Right Handle
				MarkerHit = ENotifyStateHandleHit::End;
			}
		}
	}

	return MarkerHit;
}

void SPaperZDAnimNotifyNode::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	ScreenPosition = AllottedGeometry.AbsolutePosition;
}

FVector2D SPaperZDAnimNotifyNode::ComputeDesiredSize(float) const
{
	return GetSize();
}

FVector2D SPaperZDAnimNotifyNode::GetSize() const
{
	return Size;
}

void SPaperZDAnimNotifyNode::UpdateGeometry(const FGeometry& AllottedGeometry)
{
	FTrackScaleInfo ScaleInfo(ViewInputMin.Get(), ViewInputMax.Get(), 0, 0, AllottedGeometry.Size);

	// Cache the geometry information, the alloted geometry is the same size as the track.
	CachedAllotedGeometrySize = AllottedGeometry.Size;

	NotifyTimePositionX = ScaleInfo.InputToLocalX(Notify->Time);

	if (UPaperZDAnimNotifyState* StateNotify = Cast<UPaperZDAnimNotifyState>(Notify))
	{
		NotifyDurationSizeX = ScaleInfo.PixelsPerInput * StateNotify->Duration;
	}
	else
	{
		NotifyDurationSizeX = 0.0f;
	}
	
	const TSharedRef<FSlateFontMeasure> FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	TextSize = FontMeasureService->Measure(Notify->GetDisplayName().ToString(), Font);
	LabelWidth = TextSize.X + (TextBorderSize.X * 2.f) + (ScrubHandleSize.X / 2.f);

	//Calculate scrub handle box size (the notional box around the scrub handle and the alignment marker)
	float NotifyHandleBoxWidth = FMath::Max(ScrubHandleSize.X, AlignmentMarkerSize.X * 2);

	// Work out where we will have to draw the tool tip
	float LeftEdgeToNotify = NotifyTimePositionX;
	float RightEdgeToNotify = AllottedGeometry.Size.X - NotifyTimePositionX;
	bDrawTooltipToRight = (RightEdgeToNotify > LabelWidth) || (RightEdgeToNotify > LeftEdgeToNotify);

	// Calculate widget width/position based on where we are drawing the tool tip
	Position.X = bDrawTooltipToRight ? (NotifyTimePositionX - (NotifyHandleBoxWidth / 2.f)) : (NotifyTimePositionX - LabelWidth);
	Size = bDrawTooltipToRight ? FVector2D(FMath::Max(LabelWidth, NotifyDurationSizeX), NotifyHeight) : FVector2D((LabelWidth + NotifyDurationSizeX), NotifyHeight);
	Size.X += NotifyHandleBoxWidth;

	// Widget position of the notify marker
	NotifyScrubHandleCentre = bDrawTooltipToRight ? NotifyHandleBoxWidth / 2.f : LabelWidth;
}

int32 SPaperZDAnimNotifyNode::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	int32 MarkerLayer = LayerId + 1;
	int32 ScrubHandleID = MarkerLayer + 1;
	int32 TextLayerID = ScrubHandleID + 1;

	const FSlateBrush* StyleInfo = FEditorStyle::GetBrush(TEXT("SpecialEditableTextImageNormal"));
	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(FVector2D(0, 0), AllottedGeometry.Size),
		StyleInfo,
		ESlateDrawEffect::None,
		FLinearColor::Transparent);

	//Get Name and draw it
	FText Text = FText::FromName(Notify->GetDisplayName());
	FLinearColor NodeColour = bSelected ? FLinearColor(1.0f, 0.5f, 0.0f) : FLinearColor::Red;

	float HalfScrubHandleWidth = ScrubHandleSize.X / 2.0f;
	// Show duration of AnimNotifyState
	if (NotifyDurationSizeX > 0.f)
	{
		FLinearColor BoxColor = (TrackIndex % 2) == 0 ? FLinearColor(0.0f, 1.0f, 0.5f, 0.5f) : FLinearColor(0.0f, 0.5f, 1.0f, 0.5f);
		FVector2D DurationBoxSize = FVector2D(NotifyDurationSizeX, NotifyHeight);
		FVector2D DurationBoxPosition = FVector2D(NotifyScrubHandleCentre, 0.f);
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(DurationBoxPosition, DurationBoxSize),
			StyleInfo,
			ESlateDrawEffect::None,
			BoxColor);

		DrawScrubHandle(DurationBoxPosition.X + DurationBoxSize.X, OutDrawElements, ScrubHandleID, AllottedGeometry, MyClippingRect, NodeColour);
	}

	// Background
	FVector2D LabelSize = TextSize + TextBorderSize * 2.f;
	LabelSize.X += HalfScrubHandleWidth/* + (bDrawBranchingPoint ? (BranchingPointIconSize.X + TextBorderSize.X * 2.f) : 0.f)*/;

	float LabelX = bDrawTooltipToRight ? NotifyScrubHandleCentre : NotifyScrubHandleCentre - LabelSize.X;
	float BoxHeight = (NotifyDurationSizeX > 0.f) ? (NotifyHeight - LabelSize.Y) : ((NotifyHeight - LabelSize.Y) / 2.f);

	FVector2D LabelPosition(LabelX, BoxHeight);

	FLinearColor NodeColor = Notify->Color;
	NodeColor.A = 0.5f;

	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(LabelPosition, LabelSize),
		StyleInfo,
		ESlateDrawEffect::None,
		NodeColor);

	// Frame
	// Drawing lines is slow, reserved for single selected node
	if (bSelected)
	{
		TArray<FVector2D> LinePoints;

		LinePoints.Empty();
		LinePoints.Add(LabelPosition);
		LinePoints.Add(LabelPosition + FVector2D(LabelSize.X, 0.f));
		LinePoints.Add(LabelPosition + FVector2D(LabelSize.X, LabelSize.Y));
		LinePoints.Add(LabelPosition + FVector2D(0.f, LabelSize.Y));
		LinePoints.Add(LabelPosition);

		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(),
			LinePoints,
			ESlateDrawEffect::None,
			FLinearColor::Black,
			false
		);
	}

	// Text
	FVector2D TextPosition = LabelPosition + TextBorderSize;
	if (bDrawTooltipToRight)
	{
		TextPosition.X += HalfScrubHandleWidth;
	}
	TextPosition -= FVector2D(1.f, 1.f);

	FSlateDrawElement::MakeText(
		OutDrawElements,
		TextLayerID,
		AllottedGeometry.ToPaintGeometry(TextPosition, TextSize),
		Text,
		Font,
		ESlateDrawEffect::None,
		FLinearColor::Black
	);

	NodeColor = bSelected ? FLinearColor(1.0f, 0.5f, 0.0f) : FLinearColor::Red;
	DrawScrubHandle(NotifyScrubHandleCentre, OutDrawElements, ScrubHandleID, AllottedGeometry, MyClippingRect, NodeColor);

	return TextLayerID;
}

void SPaperZDAnimNotifyNode::DrawScrubHandle(float ScrubHandleCentre, FSlateWindowElementList& OutDrawElements, int32 ScrubHandleID, const FGeometry &AllottedGeometry, const FSlateRect& MyClippingRect, FLinearColor NodeColour) const
{
	FVector2D ScrubHandlePosition(ScrubHandleCentre - ScrubHandleSize.X / 2.0f, (NotifyHeight - ScrubHandleSize.Y) / 2.f);
	FSlateDrawElement::MakeBox(
		OutDrawElements,
		ScrubHandleID,
		AllottedGeometry.ToPaintGeometry(ScrubHandlePosition, ScrubHandleSize),
		FEditorStyle::GetBrush(TEXT("Sequencer.Timeline.ScrubHandleWhole")),
		ESlateDrawEffect::None,
		NodeColour
	);
}

FReply SPaperZDAnimNotifyNode::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	bool bLeftMouseButton = MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton;
	MouseDownLastPosition = MouseEvent.GetScreenSpacePosition();

	if(bLeftMouseButton)
		return FReply::Handled().DetectDrag(SharedThis(this),EKeys::LeftMouseButton);

	return FReply::Unhandled();
}

FReply SPaperZDAnimNotifyNode::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	bool bLeftButton = MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton;
	bool bShift = MouseEvent.IsShiftDown();

	if (bLeftButton && CurrentDragHandle != ENotifyStateHandleHit::None)
	{
		// Clear the drag marker and give the mouse back
		CurrentDragHandle = ENotifyStateHandleHit::None;
		OnDeselectAllNotifies.ExecuteIfBound();

		// End drag transaction before handing mouse back
		check(DragMarkerTransactionIdx != INDEX_NONE);
		GEditor->EndTransaction();
		DragMarkerTransactionIdx = INDEX_NONE;

		return FReply::Handled().ReleaseMouseCapture();
	}
	else if (bLeftButton && !bBeingDragged) //Just clicked with no drag
	{
		if (bShift)
		{
			bSelected = !bSelected;
		}
		else
		{
			OnDeselectAllNotifies.ExecuteIfBound();
			bSelected = true;
		}
		
		OnNodeClicked.ExecuteIfBound();
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply SPaperZDAnimNotifyNode::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	// Don't do scrub handle dragging if we haven't captured the mouse.
	if (!this->HasMouseCapture()) return FReply::Unhandled();

	if (CurrentDragHandle == ENotifyStateHandleHit::None)
	{
		// We've had focus taken away - realease the mouse
		FSlateApplication::Get().ReleaseAllPointerCapture();
		return FReply::Unhandled();
	}
	
	
	FTrackScaleInfo ScaleInfo(ViewInputMin.Get(), ViewInputMax.Get(), 0, 0, CachedAllotedGeometrySize);
	UPaperZDAnimNotifyState *StateNotify = CastChecked<UPaperZDAnimNotifyState>(Notify);

	float XPositionInTrack = MyGeometry.AbsolutePosition.X - CachedTrackGeometry.AbsolutePosition.X + ScrubHandleSize.X;
	float TrackScreenSpaceXPosition = MyGeometry.AbsolutePosition.X - XPositionInTrack;

	if (CurrentDragHandle == ENotifyStateHandleHit::Start)
	{
		float OldDisplayTime = StateNotify->Time;

		float NewDisplayTime = ScaleInfo.LocalXToInput((MouseEvent.GetScreenSpacePosition() - MyGeometry.AbsolutePosition + XPositionInTrack).X);
		float NewDuration = StateNotify->Duration + OldDisplayTime - NewDisplayTime;

		StateNotify->Time = NewDisplayTime;
		StateNotify->Duration = NewDuration;
		
		//@TODO: PAN HANDLING
	}
	else
	{
		float NewDuration = ScaleInfo.LocalXToInput( (MouseEvent.GetScreenSpacePosition() - MyGeometry.AbsolutePosition + XPositionInTrack).X - ScrubHandleSize.X) - StateNotify->Time;
		//@TODO: validate delta time
		StateNotify->Duration = NewDuration;
		
		//@TODO: PAN HANDLING
	}

	//@TODO: move markers to move time

	return FReply::Handled();
}

void SPaperZDAnimNotifyNode::OnFocusLost(const FFocusEvent& InFocusEvent)
{
	if (CurrentDragHandle != ENotifyStateHandleHit::None)
	{
		// Lost focus while dragging a state node, clear the drag and end the current transaction
		CurrentDragHandle = ENotifyStateHandleHit::None;
		OnDeselectAllNotifies.ExecuteIfBound();

		check(DragMarkerTransactionIdx != INDEX_NONE);
		GEditor->EndTransaction();
		DragMarkerTransactionIdx = INDEX_NONE;
	}
}

bool SPaperZDAnimNotifyNode::SupportsKeyboardFocus() const
{
	return true;
}

FReply SPaperZDAnimNotifyNode::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FVector2D ScreenNodePosition = MyGeometry.AbsolutePosition;

	// Whether the drag has hit a duration marker
	bool bDragOnMarker = false;
	bBeingDragged = true;

	//Add Drag Markers
	if (GetDurationSize() > 0.0f)
	{
		// This is a state node, check for a drag on the markers before movement. Use last screen space position before the drag started
		// as using the last position in the mouse event gives us a mouse position after the drag was started.
		ENotifyStateHandleHit::Type MarkerHit = DurationHandleHitTest(MouseDownLastPosition);
		if (MarkerHit == ENotifyStateHandleHit::Start || MarkerHit == ENotifyStateHandleHit::End)
		{
			bDragOnMarker = true;
			bBeingDragged = false;
			CurrentDragHandle = MarkerHit;

			// Modify the owning sequence as we're now dragging the marker and begin a transaction
			check(DragMarkerTransactionIdx == INDEX_NONE);
			DragMarkerTransactionIdx = GEditor->BeginTransaction(NSLOCTEXT("AnimNotifyNode", "AnimSequenceDragTransation", "Drag State Node Marker"));
			Notify->Modify();
		}
	}

	return OnNodeDragStarted.Execute(SharedThis(this), MouseEvent, ScreenNodePosition, bDragOnMarker);
}

//////////////////////////////////////////////////////////////////////////
// SPaperZDEdTrack Implementation
void SPaperZDEdTrack::Construct(const FArguments& InArgs)
{
	PanelPtr= InArgs._AnimNotifyPanel;
	TSharedRef<SPaperZDAnimNotifyPanel> PanelRef = PanelPtr.ToSharedRef();

	this->ChildSlot
		[
			SNew(SBorder)
			.Padding(FMargin(2.0f, 2.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1)
				[
					SAssignNew(NotifyTrack, SPaperZDAnimNotifyTrack)
					.OnGetPlaybackValue(InArgs._OnGetPlaybackValue)
					.OnGetNumKeys(InArgs._OnGetNumKeys)
					.OnGetSequenceLength(InArgs._OnGetSequenceLength)
					.ViewInputMin(InArgs._ViewInputMin)
					.ViewInputMax(InArgs._ViewInputMax)
					.AnimSequence(InArgs._AnimSequence)
					.TrackIndex(InArgs._TrackIndex)
					.OnNodesDragStarted(InArgs._OnNodesDragStarted)
					.OnUpdatePanel(InArgs._OnUpdatePanel)
					.OnGetNotifyBlueprintData(InArgs._OnGetNotifyBlueprintData)
					.OnGetNotifyStateBlueprintData(InArgs._OnGetNotifyStateBlueprintData)
					.OnGetNotifyNativeClasses(InArgs._OnGetNotifyNativeClasses)
					.OnGetNotifyStateNativeClasses(InArgs._OnGetNotifyStateNativeClasses)
					.OnDeselectAllNotifies(InArgs._OnDeselectAllNotifies)
					.OnGetAllSelectedNotifyObjects(InArgs._OnGetAllSelectedNotifyObjects)
					.OnDeleteSelectedNotifies(InArgs._OnDeleteSelectedNotifies)
					.OnUnregisterCustomNotify(InArgs._OnUnregisterCustomNotify)
					.OnNodeClicked(InArgs._OnNodeClicked)
				]
				
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBox)
					.WidthOverride(InArgs._WidgetWidth)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Center)
						.FillWidth(1)
						[
							// Name of track
							SNew(STextBlock)
							.Text(FText::FromString(FString::FromInt(InArgs._TrackIndex)))
							.ColorAndOpacity(InArgs._AnimSequence->GetTrack(InArgs._TrackIndex)->Color)
						]
				
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							// Name of track
							SNew(SButton)
							.Text(LOCTEXT("AddTrackButtonLabel", "+"))
							.ToolTipText(LOCTEXT("AddTrackTooltip", "Add track above here"))
							.OnClicked(PanelRef, &SPaperZDAnimNotifyPanel::InsertTrack, InArgs._TrackIndex + 1)
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							// Name of track
							SNew(SButton)
							.Text(LOCTEXT("RemoveTrackButtonLabel", "-"))
							.ToolTipText(LOCTEXT("RemoveTrackTooltip", "Remove this track"))
							.OnClicked(PanelRef, &SPaperZDAnimNotifyPanel::DeleteTrack, InArgs._TrackIndex)
							.IsEnabled(PanelRef->CanDeleteTrack(InArgs._TrackIndex))
						]
					]
				]
			]
		];
}

//////////////////////////////////////////////////////////////////////////
// SPaperZDAnimNotifyTrack Implementation
void SPaperZDAnimNotifyTrack::Construct(const FArguments& InArgs)
{
	OnGetPlaybackValue = InArgs._OnGetPlaybackValue;
	OnGetNumKeys = InArgs._OnGetNumKeys;
	OnGetSequenceLength = InArgs._OnGetSequenceLength;
	OnNodesDragStarted = InArgs._OnNodesDragStarted;
	OnUpdatePanel = InArgs._OnUpdatePanel;
	OnGetNotifyNativeClasses = InArgs._OnGetNotifyNativeClasses;
	OnGetNotifyStateNativeClasses = InArgs._OnGetNotifyStateNativeClasses;
	OnGetNotifyBlueprintData = InArgs._OnGetNotifyBlueprintData;
	OnGetNotifyStateBlueprintData = InArgs._OnGetNotifyStateBlueprintData;
	OnDeselectAllNotifies = InArgs._OnDeselectAllNotifies;
	OnGetAllSelectedNotifyObjects = InArgs._OnGetAllSelectedNotifyObjects;
	OnDeleteSelectedNotifies = InArgs._OnDeleteSelectedNotifies;
	OnUnregisterCustomNotify = InArgs._OnUnregisterCustomNotify;
	OnNodeClicked = InArgs._OnNodeClicked;

	ViewInputMin = InArgs._ViewInputMin;
	ViewInputMax = InArgs._ViewInputMax;
	AnimSequence = InArgs._AnimSequence;
	TrackIndex = InArgs._TrackIndex;

	this->ChildSlot
		[
			SAssignNew(TrackArea, SBorder)
			.BorderImage(FEditorStyle::GetBrush("NoBorder"))
		.Padding(FMargin(0.f, 0.f))
		];

	Update();
}

bool SPaperZDAnimNotifyTrack::HasInvalidNodes()
{
	for (TSharedPtr<SPaperZDAnimNotifyNode> Node : NotifyNodes)
	{
		if (!Node->Notify.IsValid())
			return true;

		UPaperZDAnimNotifyCustom* CustomNotify = Cast<UPaperZDAnimNotifyCustom>(Node->Notify.Get());

		if (CustomNotify && !AnimSequence->GetAnimBP()->RegisteredNotifyNames.Contains(CustomNotify->Name))
			return true;
	}

	return false;
}

void SPaperZDAnimNotifyTrack::DeselectAllNodes()
{
	for (TSharedPtr<SPaperZDAnimNotifyNode> Node : NotifyNodes)
	{
		Node->bSelected = false;
	}
}

void SPaperZDAnimNotifyTrack::HandleNodeDrop(TSharedPtr<SPaperZDAnimNotifyNode> Node, FVector2D AbsolutePosition)
{
	const FScopedTransaction Transaction(LOCTEXT("DropNotify", "Move Notify"));
	AnimSequence->Modify();
	AnimSequence->MoveNotify(Node->Notify.Get(), TrackIndex);

	FTrackScaleInfo ScaleInfo(ViewInputMin.Get(), ViewInputMax.Get(), 0.f, 0.f, CachedGeometry.Size);
	Node->Notify->Modify();
	Node->Notify->Time = ScaleInfo.LocalXToInput(AbsolutePosition.X - CachedGeometry.AbsolutePosition.X);
}

FReply SPaperZDAnimNotifyTrack::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	bool bLeftMouseButton = MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton;
	bool bRightMouseButton = MouseEvent.GetEffectingButton() == EKeys::RightMouseButton;
	bool bShift = MouseEvent.IsShiftDown();
	bool bCtrl = MouseEvent.IsControlDown();

	if (bRightMouseButton)
	{
		//Find the hit notify (if any)
		int index = INDEX_NONE;
		for (int i = 0; i < NotifyNodes.Num(); i++)
		{
			TSharedPtr<SPaperZDAnimNotifyNode> Node = NotifyNodes[i];
			int32 HitInfo;
			if (Node->HitTest(MouseEvent.GetScreenSpacePosition(), HitInfo))
			{
				index = i;
				break;
			}
		}

		bool bNodeHit = false;
		if (index != INDEX_NONE)
		{
			TSharedPtr<SPaperZDAnimNotifyNode> Node = NotifyNodes[index];
			
			//If the node wasn't selected it means we are clicking on something that was outside the possible multi-selection, and that multi selection is now invalid
			if (!Node->bSelected)
			{
				OnDeselectAllNotifies.ExecuteIfBound();
				Node->bSelected = true;
			}

			bNodeHit = true;
		}


		TSharedPtr<SWidget> Menu = GetContextMenu(MyGeometry, MouseEvent, bNodeHit);
		return (Menu.IsValid())
			? FReply::Handled().ReleaseMouseCapture().SetUserFocus(Menu.ToSharedRef(), EFocusCause::SetDirectly)
			: FReply::Handled().ReleaseMouseCapture();
	}
	else if (bLeftMouseButton)
	{
		OnDeselectAllNotifies.ExecuteIfBound();
	}

	return FReply::Unhandled();
}

float SPaperZDAnimNotifyTrack::CalculateTime(const FGeometry& MyGeometry, FVector2D Position, bool bInputIsAbsolute)
{
	if (bInputIsAbsolute)
	{
		Position = MyGeometry.AbsoluteToLocal(Position);
	}
	FTrackScaleInfo ScaleInfo(ViewInputMin.Get(), ViewInputMax.Get(), 0, 0, MyGeometry.Size);
	return FMath::Clamp<float>(ScaleInfo.LocalXToInput(Position.X), 0.f, CachedGeometry.Size.X);
}

TSharedPtr<SWidget> SPaperZDAnimNotifyTrack::GetContextMenu(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent, const bool bHitNode)
{
	FVector2D CursorPos = MouseEvent.GetScreenSpacePosition();

	//Necessary for knowing where to put the notify that was created
	LastClickedTime = CalculateTime(MyGeometry, MouseEvent.GetScreenSpacePosition());

	FMenuBuilder MenuBuilder(true, MakeShareable(new FUICommandList));

	if (!bHitNode) { //@TODO: change this when we're be able to replace notifies
		MenuBuilder.BeginSection("AnimNotify", LOCTEXT("NotifyHeading", "Notify"));
		MenuBuilder.AddSubMenu(
			NSLOCTEXT("NewNotifySubMenu", "NewNotifySubMenuAddNotify", "Add Notify..."),
			NSLOCTEXT("NewNotifySubMenu", "NewNotifySubMenuAddNotifyToolTip", "Add AnimNotifyEvent"),
			FNewMenuDelegate::CreateRaw(this, &SPaperZDAnimNotifyTrack::FillNewNotifyMenu, bHitNode));

		MenuBuilder.AddSubMenu(
			NSLOCTEXT("NewNotifySubMenu", "NewNotifySubMenuAddNotifyState", "Add Notify State..."),
			NSLOCTEXT("NewNotifySubMenu", "NewNotifySubMenuAddNotifyStateToolTip", "Add AnimNotifyState"),
			FNewMenuDelegate::CreateRaw(this, &SPaperZDAnimNotifyTrack::FillNewNotifyStateMenu, bHitNode));
		MenuBuilder.EndSection();
	}

	if (bHitNode) //@TODO: Not using else because top if should be removed when supporting ReplaceNotifies
	{
		TArray<UPaperZDAnimNotify_Base*>SelectedNotifies = OnGetAllSelectedNotifyObjects.Execute();

		//Create common commands
		MenuBuilder.BeginSection("AnimNotifyManage", LOCTEXT("NotifyManageHeading", "Manage"));
		MenuBuilder.AddMenuEntry(
			NSLOCTEXT("DeleteNotify", "DeleteNotify", "Delete"),
			NSLOCTEXT("DeleteNotify", "DeleteNotifyTooltip", "Delete the selected notifies"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SPaperZDAnimNotifyTrack::DeleteSelectedNotifies)));

		//@TODO: Add more commands (edit, cut, etc)

		//Only if one custom notify (no multi select)
		if (SelectedNotifies.Num() == 1)
		{
			UPaperZDAnimNotifyCustom *Notify = Cast<UPaperZDAnimNotifyCustom>(SelectedNotifies[0]);

			if(Notify)
				MenuBuilder.AddMenuEntry(
					NSLOCTEXT("UnregisterCustomNotify", "UnregisterCustomNotify", "Unregister Custom Notify"),
					NSLOCTEXT("UnregisterCustomNotify", "UnregisterCustomNotifyTooltip", "Delete this notify and unregister it from the AnimInstance, removing any notify of this type on any other state. Cannot be undone!"),
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateSP(this, &SPaperZDAnimNotifyTrack::UnregisterCustomNotify,Notify)));
		}

		MenuBuilder.EndSection();
	}

	FWidgetPath WidgetPath = MouseEvent.GetEventPath() != nullptr ? *MouseEvent.GetEventPath() : FWidgetPath();

	// Display the newly built menu
	FSlateApplication::Get().PushMenu(SharedThis(this), WidgetPath, MenuBuilder.MakeWidget(), CursorPos, FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));

	return TSharedPtr<SWidget>();
}

void SPaperZDAnimNotifyTrack::DeleteSelectedNotifies()
{
	OnDeleteSelectedNotifies.ExecuteIfBound();
}

void SPaperZDAnimNotifyTrack::UnregisterCustomNotify(UPaperZDAnimNotifyCustom *Notify)
{
	TSharedRef<SPaperZDConfirmDialog> Dialog = SNew(SPaperZDConfirmDialog).DetailText(LOCTEXT("PaperZD_ConfirmUnregister", "Are you sure you want to unregister this notify?. It will delete every notify of this type. \rCannot be undone!"));

	if (Dialog->Show())
	{
		OnUnregisterCustomNotify.ExecuteIfBound(Notify);
	}
}

void SPaperZDAnimNotifyTrack::GetNotifyMenuData(TArray<FAssetData>& NotifyAssetData, TArray<BlueprintNotifyMenuInfo>& OutNotifyMenuData)
{
	for (FAssetData& NotifyData : NotifyAssetData)
	{
		OutNotifyMenuData.AddZeroed();
		BlueprintNotifyMenuInfo& MenuInfo = OutNotifyMenuData.Last();

		MenuInfo.BlueprintPath = NotifyData.ObjectPath.ToString();

		//Parse the name
		FString TempNotifyName = NotifyData.AssetName.ToString();
		TempNotifyName = TempNotifyName.Replace(TEXT("AnimNotify_"), TEXT(""), ESearchCase::CaseSensitive);
		TempNotifyName = TempNotifyName.Replace(TEXT("AnimNotifyState_"), TEXT(""), ESearchCase::CaseSensitive);
		MenuInfo.NotifyName = FName(*TempNotifyName);

		// this functionality is only available in native class
		// so we don't have to call BP function but just call native on the check of validity
		FString NativeParentClassName;
		if (NotifyData.GetTagValue("NativeParentClass", NativeParentClassName))
		{
			UObject* Outer = nullptr;
			ResolveName(Outer, NativeParentClassName, false, false);
			MenuInfo.BaseClass = FindObject<UClass>(ANY_PACKAGE, *NativeParentClassName);
		}
	}

	OutNotifyMenuData.Sort([](const BlueprintNotifyMenuInfo& A, const BlueprintNotifyMenuInfo& B)
	{
		return A.NotifyName.LexicalLess(B.NotifyName);
	});
}

void SPaperZDAnimNotifyTrack::FillNewNotifyMenu(FMenuBuilder& MenuBuilder, bool bIsReplaceWithMenu)
{
	TArray<UClass*> NativeNotifyClasses;
	OnGetNotifyNativeClasses.ExecuteIfBound(NativeNotifyClasses);

	TArray<FAssetData> NotifyAssetData;
	TArray<BlueprintNotifyMenuInfo> NotifyMenuData;
	OnGetNotifyBlueprintData.ExecuteIfBound(NotifyAssetData);
	GetNotifyMenuData(NotifyAssetData, NotifyMenuData);

	for (BlueprintNotifyMenuInfo& NotifyData : NotifyMenuData)
	{
		const FText LabelText = FText::FromName(NotifyData.NotifyName);

		FUIAction UIAction;
		FText Description = FText::GetEmpty();
		if (!bIsReplaceWithMenu)
		{
			Description = LOCTEXT("NewNotifySubMenu_ToolTip", "Add an existing notify");
			UIAction.ExecuteAction.BindRaw(
				this, &SPaperZDAnimNotifyTrack::CreateNewBlueprintNotifyAtCursor,
				NotifyData.NotifyName,
				NotifyData.BlueprintPath);
			UIAction.CanExecuteAction.BindRaw(
				this, &SPaperZDAnimNotifyTrack::IsValidToPlace,
				NotifyData.BaseClass);
		}
		else
		{
			//@TODO: Replace Notify
		}

		MenuBuilder.AddMenuEntry(LabelText, Description, FSlateIcon(), UIAction);
	}
	
	MenuBuilder.BeginSection("NativeNotifies", LOCTEXT("NewNotifyMenu_Native", "Native Notifies"));
	{
		for (UClass* Class : NativeNotifyClasses)
		{
			if (Class->HasAllClassFlags(CLASS_Abstract))
			{
				continue; // skip abstract classes
			}

			const FText LabelText = Class->GetDisplayNameText();
			const FName LabelName = FName(*LabelText.ToString());

			FUIAction UIAction;
			FText Description = FText::GetEmpty();
			if (!bIsReplaceWithMenu)
			{
				Description = LOCTEXT("NewNotifySubMenu_NativeToolTip", "Add an existing native notify");
				UIAction.ExecuteAction.BindRaw(
					this, &SPaperZDAnimNotifyTrack::CreateNewNotifyAtCursor,
					LabelName,
					Class);
				UIAction.CanExecuteAction.BindRaw(
					this, &SPaperZDAnimNotifyTrack::IsValidToPlace,
					Class);
			}
			else
			{
				//@TODO: Replace Notify
			}
			MenuBuilder.AddMenuEntry(LabelText, Description, FSlateIcon(), UIAction);
		}
	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection("AnimNotifyCustom", LOCTEXT("NewNotifySubMenu_Custom", "Custom"));
	{
		UPaperZDAnimBP* AnimBP = AnimSequence->GetAnimBP();
		if (AnimBP)
		{
			for (int i = 0; i < AnimBP->RegisteredNotifyNames.Num(); i++)
			{
				FName NotifyName = AnimBP->RegisteredNotifyNames[i];

				FText Description = FText::GetEmpty();
				if (!bIsReplaceWithMenu)
				{
					Description = LOCTEXT("NewNotifySubMenu_ToolTip", "Add an existing notify");
				}
				else
				{
					Description = LOCTEXT("ReplaceWithNotifySubMenu_ToolTip", "Replace with an existing notify");
				}

				FUIAction UIAction;
				if (!bIsReplaceWithMenu)
				{
					UIAction.ExecuteAction.BindRaw(
						this, &SPaperZDAnimNotifyTrack::CreateNewNotifyAtCursor,
						NotifyName,
						UPaperZDAnimNotifyCustom::StaticClass()); //(UClass*)nullptr
				}
				else
				{
					//@TODO: Replace Notify
				}

				MenuBuilder.AddMenuEntry(FText::FromName(NotifyName), Description, FSlateIcon(), UIAction);
			}
		}
	}
	MenuBuilder.EndSection();

	if (!bIsReplaceWithMenu)
	{
		MenuBuilder.BeginSection("AnimNotifyCreateNew");
		{
			FUIAction UIAction;
			UIAction.ExecuteAction.BindRaw(
				this, &SPaperZDAnimNotifyTrack::OnNewNotifyClicked);
			MenuBuilder.AddMenuEntry(LOCTEXT("NewNotify", "New Custom Notify"), LOCTEXT("NewNotifyToolTip", "Create a new animation notify"), FSlateIcon(), UIAction);
		}
		MenuBuilder.EndSection();
	}
}

void SPaperZDAnimNotifyTrack::FillNewNotifyStateMenu(FMenuBuilder& MenuBuilder, bool bIsReplaceWithMenu)
{
	TArray<UClass*> NativeNotifyClasses;
	OnGetNotifyStateNativeClasses.ExecuteIfBound(NativeNotifyClasses);

	TArray<FAssetData> NotifyAssetData;
	TArray<BlueprintNotifyMenuInfo> NotifyMenuData;
	OnGetNotifyStateBlueprintData.ExecuteIfBound(NotifyAssetData);
	GetNotifyMenuData(NotifyAssetData, NotifyMenuData);
	
	for (BlueprintNotifyMenuInfo& NotifyData : NotifyMenuData)
	{
		const FText LabelText = FText::FromName(NotifyData.NotifyName);

		FUIAction UIAction;
		FText Description = FText::GetEmpty();
		if (!bIsReplaceWithMenu)
		{
			Description = LOCTEXT("NewNotifySubMenu_ToolTip", "Add an existing notify");
			UIAction.ExecuteAction.BindRaw(
				this, &SPaperZDAnimNotifyTrack::CreateNewBlueprintNotifyAtCursor,
				NotifyData.NotifyName,
				NotifyData.BlueprintPath);
			UIAction.CanExecuteAction.BindRaw(
				this, &SPaperZDAnimNotifyTrack::IsValidToPlace,
				NotifyData.BaseClass);
		}
		else
		{
			//@TODO: Replace Notify
		}

		MenuBuilder.AddMenuEntry(LabelText, Description, FSlateIcon(), UIAction);
	}
	
	MenuBuilder.BeginSection("NativeNotifies", LOCTEXT("NewNotifyMenu_Native", "Native Notifies"));
	{
		for (UClass* Class : NativeNotifyClasses)
		{
			if (Class->HasAllClassFlags(CLASS_Abstract))
			{
				continue; // skip abstract classes
			}

			const FText LabelText = Class->GetDisplayNameText();
			const FName LabelName = FName(*LabelText.ToString());

			FUIAction UIAction;
			FText Description = FText::GetEmpty();
			if (!bIsReplaceWithMenu)
			{
				Description = LOCTEXT("NewNotifySubMenu_NativeToolTip", "Add an existing native notify");
				UIAction.ExecuteAction.BindRaw(
					this, &SPaperZDAnimNotifyTrack::CreateNewNotifyAtCursor,
					LabelName,
					Class);
				UIAction.CanExecuteAction.BindRaw(
					this, &SPaperZDAnimNotifyTrack::IsValidToPlace,
					Class);
			}
			else
			{
				//@TODO: Replace Notify
			}

			MenuBuilder.AddMenuEntry(LabelText, Description, FSlateIcon(), UIAction);
		}
	}
	MenuBuilder.EndSection();
}
void SPaperZDAnimNotifyTrack::OnNewNotifyClicked()
{
	// Show dialog to enter new track name
	TSharedRef<STextEntryPopup> TextEntry =
		SNew(STextEntryPopup)
		.Label(LOCTEXT("NewNotifyLabel", "Notify Name"))
		.OnTextCommitted(this, &SPaperZDAnimNotifyTrack::AddNewNotify);
	
	// Show dialog to enter new event name
	FSlateApplication::Get().PushMenu(
		AsShared(), // Menu being summoned from a menu that is closing: Parent widget should be k2 not the menu thats open or it will be closed when the menu is dismissed
		FWidgetPath(),
		TextEntry,
		FSlateApplication::Get().GetCursorPos(),
		FPopupTransitionEffect(FPopupTransitionEffect::TypeInPopup)
	);
}

void SPaperZDAnimNotifyTrack::AddNewNotify(const FText& NewNotifyName, ETextCommit::Type CommitInfo)
{
	if ((CommitInfo == ETextCommit::OnEnter)/* && AnimSequence->AnimInstance*/) //@TODO: check why did i do this?
	{
		const FScopedTransaction Transaction(LOCTEXT("AddNewNotifyEvent", "Add New Anim Notify"));
		FName NewName = FName(*NewNotifyName.ToString());
		AnimSequence->GetAnimBP()->RegisteredNotifyNames.AddUnique(NewName);
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(AnimSequence->GetAnimBP());

		CreateNewNotifyAtCursor(NewName, UPaperZDAnimNotifyCustom::StaticClass()); 
	}

	FSlateApplication::Get().DismissAllMenus();
}

void SPaperZDAnimNotifyTrack::CreateNewBlueprintNotifyAtCursor(FName NewNotifyName, FString BlueprintPath)
{
	TSubclassOf<UObject> BlueprintClass = GetBlueprintClassFromPath(BlueprintPath);
	check(BlueprintClass);
	CreateNewNotifyAtCursor(NewNotifyName, BlueprintClass);
}

void SPaperZDAnimNotifyTrack::CreateNewNotifyAtCursor(FName NewNotifyName, UClass* NotifyClass)
{
	const FScopedTransaction Transaction(LOCTEXT("AddNotifyEvent", "Add Anim Notify"));
	AnimSequence->Modify();
	AnimSequence->AddNotifyToTrack(NotifyClass, TrackIndex, NewNotifyName, LastClickedTime);
	OnUpdatePanel.ExecuteIfBound();
}

bool SPaperZDAnimNotifyTrack::IsValidToPlace(UClass* NotifyClass) const
{
	//@TODO: Change this when more rules are added
	return true;
}

void SPaperZDAnimNotifyTrack::DisconnectSelectedNodesForDrag(TArray<TSharedPtr<SPaperZDAnimNotifyNode>>& DragNodes)
{
	for (int i = 0; i < NotifyNodes.Num(); i++)
	{
		TSharedPtr<SPaperZDAnimNotifyNode> Node = NotifyNodes[i];

		if (Node->bSelected)
		{
			NodeOverlay->RemoveSlot(Node->AsShared());

			DragNodes.Add(Node);
		}
	}
}

int32 SPaperZDAnimNotifyTrack::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const FSlateBrush* StyleInfo = FEditorStyle::GetBrush(TEXT("Persona.NotifyEditor.NotifyTrackBackground"));
	FLinearColor Color = AnimSequence->GetTrack(TrackIndex)->Color;

	FPaintGeometry MyGeometry = AllottedGeometry.ToPaintGeometry();
	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId,
		MyGeometry,
		StyleInfo,
		ESlateDrawEffect::None,
		Color
	);

	int32 CustomLayerId = LayerId + 1;

	// draw line for every 1/4 length
	FTrackScaleInfo ScaleInfo(ViewInputMin.Get(), ViewInputMax.Get(), 0.f, 0.f, AllottedGeometry.Size);

	const bool bCanDisplayFrames = OnGetNumKeys.IsBound() && OnGetSequenceLength.IsBound();
	if (bCanDisplayFrames)
	{
		int32 Divider =  SScrubWidget::GetDivider(ViewInputMin.Get(), ViewInputMax.Get(), AllottedGeometry.Size, OnGetSequenceLength.Execute(), OnGetNumKeys.Execute());
		const FAnimKeyHelper Helper(OnGetSequenceLength.Execute(), OnGetNumKeys.Execute());

		float TimePerKey = Helper.TimePerKey();
		for (int32 I = 1; I<Helper.GetNumKeys() - 1; ++I)
		{
			if (I % Divider == 0)
			{
				float XPos = ScaleInfo.InputToLocalX(TimePerKey*I);

				TArray<FVector2D> LinePoints;
				LinePoints.Add(FVector2D(XPos, 0.f));
				LinePoints.Add(FVector2D(XPos, AllottedGeometry.Size.Y));

				FSlateDrawElement::MakeLines(
					OutDrawElements,
					CustomLayerId,
					MyGeometry,
					LinePoints,
					ESlateDrawEffect::None,
					FLinearColor::Black
				);
			}
		}
	}

	++CustomLayerId;

	for (int32 I = 0; I<NotifyNodes.Num(); ++I)
	{
		NotifyNodes[I].Get()->UpdateGeometry(AllottedGeometry);
	}
	++CustomLayerId;

	float Value = 0.f;

	if (OnGetPlaybackValue.IsBound())
	{
		Value = OnGetPlaybackValue.Execute();
	}

	{
		float XPos = ScaleInfo.InputToLocalX(Value);

		TArray<FVector2D> LinePoints;
		LinePoints.Add(FVector2D(XPos, 0.f));
		LinePoints.Add(FVector2D(XPos, AllottedGeometry.Size.Y));


		FSlateDrawElement::MakeLines(
			OutDrawElements,
			CustomLayerId,
			MyGeometry,
			LinePoints,
			ESlateDrawEffect::None,
			FLinearColor::Red
		);
	}

	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyClippingRect, OutDrawElements, CustomLayerId, InWidgetStyle, bParentEnabled);
}

bool SPaperZDAnimNotifyTrack::HitTest(FVector2D TestPoint, int32 &HitInfo)
{
	//Init the variable
	HitInfo = EHP_Hit;

	FVector2D ScreenPosition = CachedGeometry.AbsolutePosition;
	FVector2D Size = CachedGeometry.GetLocalSize();
	FVector2D LowerRightPoint = ScreenPosition + Size;

	//If hit
	if (TestPoint >= ScreenPosition && TestPoint <= LowerRightPoint)
	{
		return true;
	}
	
	//No hit ocurred, check horizontal limits first
	if (TestPoint.X < ScreenPosition.X)
		HitInfo |= EHP_Left;
	else if (TestPoint.X > LowerRightPoint.X)
		HitInfo |= EHP_Right;

	//Check Vertical
	if (TestPoint.Y > ScreenPosition.Y)
		HitInfo |= EHP_Below;
	else if (TestPoint.Y < LowerRightPoint.Y)
		HitInfo |= EHP_Above;

	return false;
}

void SPaperZDAnimNotifyTrack::Update()
{
	NotifyNodes.Empty();

	TrackArea->SetContent(
		SAssignNew(NodeOverlay, SOverlay)
	);

	//@PATCH: this is for avoiding having a non updated track when reinstancing a blueprint notify
	UPaperZDAnimTrack *Track = AnimSequence->GetTrack(TrackIndex);
	for (int32 i = 0; i < Track->AnimNotifies.Num(); i++) {
		TSharedPtr<SPaperZDAnimNotifyNode> Node;
		
		if (!ensure(Track->AnimNotifies[i]->IsValidLowLevel())) //Test to check if the Notifies are Ever non valid after an undo or deletion
			continue;

		SAssignNew(Node, SPaperZDAnimNotifyNode)
			.ViewInputMin(ViewInputMin)
			.ViewInputMax(ViewInputMax)
			.Notify(Track->AnimNotifies[i])
			.TrackIndex(TrackIndex)
			.OnNodeDragStarted(this, &SPaperZDAnimNotifyTrack::OnNotifyNodeDragStarted, i)
			.OnDeselectAllNotifies(OnDeselectAllNotifies)
			.OnNodeClicked(OnNodeClicked);

		NodeOverlay->AddSlot()
			.Padding(TAttribute<FMargin>::Create(TAttribute<FMargin>::FGetter::CreateSP(this, &SPaperZDAnimNotifyTrack::GetNotifyTrackPadding, i)))
			.VAlign(VAlign_Center)
			[
				Node->AsShared()
			];

		NotifyNodes.Add(Node);
	}
}

FReply SPaperZDAnimNotifyTrack::OnNotifyNodeDragStarted(TSharedRef<SPaperZDAnimNotifyNode> NotifyNode, const FPointerEvent& MouseEvent, const FVector2D& ScreenNodePosition, const bool bDragOnMarker, int32 NotifyIndex)
{
	//Make sure that this node is selected
	if (!NotifyNode->bSelected)
	{
		NotifyNode->bSelected = true;
	}

	// If we're dragging one of the direction markers we don't need to call any further as we don't want the drag drop op
	if (!bDragOnMarker)
	{
		return OnNodesDragStarted.Execute(MouseEvent.GetScreenSpacePosition(), bDragOnMarker);
	}
	else
	{
		// Capture the mouse in the node
		return FReply::Handled().CaptureMouse(NotifyNode).UseHighPrecisionMouseMovement(NotifyNode);
	}
}

//////////////////////////////////////////////////////////////////////////
// FPaperZDAnimNotifyPanelCommands
FPaperZDAnimNotifyPanelCommands::FPaperZDAnimNotifyPanelCommands() : TCommands<FPaperZDAnimNotifyPanelCommands>("AnimNotifyPanel", NSLOCTEXT("Contexts", "AnimNotifyPanel", "Anim Notify Panel"), NAME_None, FEditorStyle::GetStyleSetName())
{
}

void FPaperZDAnimNotifyPanelCommands::RegisterCommands()
{
	UI_COMMAND(DeleteNotify, "Delete", "Deletes the selected notifies.", EUserInterfaceActionType::Button, FInputChord(EKeys::Platform_Delete));
}

//////////////////////////////////////////////////////////////////////////
// SPaperZDAnimNotifyPanel

void SPaperZDAnimNotifyPanel::Construct(const FArguments& InArgs)
{
	WidgetWidth = InArgs._WidgetWidth;
	ViewInputMin = InArgs._ViewInputMin;
	ViewInputMax = InArgs._ViewInputMax;
	AnimSequence = InArgs._AnimSequence;
	
	OnGetPlaybackValue = InArgs._OnGetPlaybackValue;
	OnGetNumKeys = InArgs._OnGetNumKeys;
	OnGetSequenceLength = InArgs._OnGetSequenceLength;
	OnSetViewRange = InArgs._OnSetViewRange;
	OnNotifySelectionChanged = InArgs._OnNotifySelectionChanged;

	this->ChildSlot
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			[
				SNew(SExpandableArea).
				AreaTitle(LOCTEXT("Notifies", "Notifies"))
				.AddMetaData<FTagMetaData>(TEXT("PaperZDAnimNotify.Notify"))
				.BodyContent()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					[
						SAssignNew(PanelArea, SBorder)
						.BorderImage(FEditorStyle::GetBrush("NoBorder"))
						.Padding(FMargin(2.0f, 2.0f))
						.ColorAndOpacity(FLinearColor::White)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1)
						[
							SAssignNew(NotifyTrackScrollBar, SScrollBar)
							.Orientation(EOrientation::Orient_Horizontal)
							.AlwaysShowScrollbar(true)
							.OnUserScrolled(this, &SPaperZDAnimNotifyPanel::OnNotifyTrackScrolled)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SBox)
							.WidthOverride(InArgs._WidgetWidth)
						]
					]
				]
			]
		];

	FPaperZDAnimNotifyPanelCommands::Register();
	BindCommands();

	//Bind PostEditChanges
	AnimSequence->OnPostEditUndo.BindSP(SharedThis(this), &SPaperZDAnimNotifyPanel::OnPostUndo);

	//Check for track init
	AnimSequence->InitTracks();

	//Configure the ViewRange of the slider
	InputViewRangeChanged(ViewInputMin.Get(), ViewInputMax.Get());

	//Update the View
	Update();
}

SPaperZDAnimNotifyPanel::~SPaperZDAnimNotifyPanel()
{
	AnimSequence->OnPostEditUndo.Unbind();
}

void SPaperZDAnimNotifyPanel::OnNotifyTrackScrolled(float InScrollOffsetFraction)
{
	if (!OnGetSequenceLength.IsBound())
		return;

	float Ratio = (ViewInputMax.Get() - ViewInputMin.Get()) / OnGetSequenceLength.Execute();
	float MaxOffset = (Ratio < 1.0f) ? 1.0f - Ratio : 0.0f;
	InScrollOffsetFraction = FMath::Clamp(InScrollOffsetFraction, 0.0f, MaxOffset);

	// Calculate new view ranges
	float min = InScrollOffsetFraction * OnGetSequenceLength.Execute();
	float max = (InScrollOffsetFraction + Ratio) * OnGetSequenceLength.Execute();

	InputViewRangeChanged(min, max);

	//Call Delegate
	if (OnSetViewRange.IsBound())
		OnSetViewRange.Execute(min, max);
}

void SPaperZDAnimNotifyPanel::Update()
{
	RefreshNotifyTracks();
}

void SPaperZDAnimNotifyPanel::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	//Always check if any notify is invalid (specially custom notifies)
	for (TSharedPtr<SPaperZDAnimNotifyTrack> Track : NotifyTracks)
	{
		if (Track->HasInvalidNodes()) {
			AnimSequence->CleanInvalidNodes(AnimSequence->GetAnimBP()->RegisteredNotifyNames);
			Update();
			return;
		}
	}
}

void SPaperZDAnimNotifyPanel::RefreshNotifyTracks()
{
	check(AnimSequence.IsValid());

	NotifyTracks.Empty();

	TSharedPtr<SVerticalBox> NotifySlots;
	PanelArea->SetContent(
		SAssignNew(NotifySlots, SVerticalBox)
	);

	//NotifyAnimTracks.Empty();
	TArray<UPaperZDAnimTrack *> Tracks = AnimSequence->GetTracks();
	for (int32 i = Tracks.Num() - 1; i >= 0; --i)
	{
		UPaperZDAnimTrack *Track = Tracks[i];
		TSharedPtr<SPaperZDEdTrack> EdTrack;

		NotifySlots->AddSlot()
			.AutoHeight()
			.VAlign(VAlign_Center)
			[
				SAssignNew(EdTrack, SPaperZDEdTrack)
				.TrackIndex(i)
				.AnimSequence(AnimSequence.Get())
				.AnimNotifyPanel(SharedThis(this))
				.WidgetWidth(WidgetWidth)
				.OnGetPlaybackValue(OnGetPlaybackValue)
				.OnGetNumKeys(OnGetNumKeys)
				.OnGetSequenceLength(OnGetSequenceLength)
				.ViewInputMin(ViewInputMin)
				.ViewInputMax(ViewInputMax)
				.OnNodesDragStarted(this, &SPaperZDAnimNotifyPanel::OnNotifyNodesDragStarted)
				.OnUpdatePanel(this, &SPaperZDAnimNotifyPanel::Update)
				.OnGetNotifyBlueprintData(this, &SPaperZDAnimNotifyPanel::OnGetNotifyBlueprintData, &NotifyClassNames)
				.OnGetNotifyStateBlueprintData(this, &SPaperZDAnimNotifyPanel::OnGetNotifyBlueprintData, &NotifyStateClassNames)
				.OnGetNotifyNativeClasses(this, &SPaperZDAnimNotifyPanel::OnGetNativeNotifyData, UPaperZDAnimNotify::StaticClass(), &NotifyClassNames)
				.OnGetNotifyStateNativeClasses(this, &SPaperZDAnimNotifyPanel::OnGetNativeNotifyData, UPaperZDAnimNotifyState::StaticClass(), &NotifyStateClassNames)
				.OnDeselectAllNotifies(this, &SPaperZDAnimNotifyPanel::DeselectAllNotifies)
				.OnGetAllSelectedNotifyObjects(this, &SPaperZDAnimNotifyPanel::GetAllSelectedNotifyObjects)
				.OnDeleteSelectedNotifies(this, &SPaperZDAnimNotifyPanel::DeleteSelectedNodes)
				.OnUnregisterCustomNotify(this, &SPaperZDAnimNotifyPanel::UnregisterSelectedCustomNotify)
				.OnNodeClicked(this, &SPaperZDAnimNotifyPanel::OnNodeClicked)
			];

		NotifyTracks.Add(EdTrack->NotifyTrack);
	}
}

void SPaperZDAnimNotifyPanel::OnNodeClicked()
{
	OnNotifySelectionChanged.ExecuteIfBound(GetAllSelectedNotifyObjects<UObject>());
}

void SPaperZDAnimNotifyPanel::OnPostUndo()
{
	Update();
}

void SPaperZDAnimNotifyPanel::InputViewRangeChanged(float ViewMin, float ViewMax)
{
	if (!OnGetSequenceLength.IsBound())
		return;
	float Ratio = (ViewMax - ViewMin) / OnGetSequenceLength.Execute();
	float OffsetFraction = ViewMin / OnGetSequenceLength.Execute();
	NotifyTrackScrollBar->SetState(OffsetFraction, Ratio);
}

FReply SPaperZDAnimNotifyPanel::InsertTrack(int32 InsertInto)
{
	const FScopedTransaction Transaction(LOCTEXT("AddNotifyTrack", "Add AnimTrack"));
	AnimSequence->Modify();
	AnimSequence->CreateTrack(InsertInto); //@TODO: TEST
	RefreshNotifyTracks();
	return FReply::Handled();
}

FReply SPaperZDAnimNotifyPanel::DeleteTrack(int32 TrackIndex)
{
	const FScopedTransaction Transaction(LOCTEXT("DeleteNotifyTrack", "Delete AnimTrack"));
	AnimSequence->Modify();
	AnimSequence->RemoveTrack(TrackIndex);
	RefreshNotifyTracks();
	return FReply::Handled();
}

bool SPaperZDAnimNotifyPanel::CanDeleteTrack(int TrackIndex)
{
	return AnimSequence->GetTracks().Num() > 1;
}

void SPaperZDAnimNotifyPanel::OnGetNativeNotifyData(TArray<UClass*>& OutClasses, UClass* NotifyOutermost, TArray<FString>* OutAllowedBlueprintClassNames)
{
	for (TObjectIterator<UClass> It; It; ++It)
	{
		UClass* Class = *It;

		if (Class->IsChildOf(NotifyOutermost) && Class->HasAllClassFlags(CLASS_Native) && !Class->IsInBlueprint() && !Class->IsChildOf(UPaperZDAnimNotifyCustom::StaticClass()))
		{
			OutClasses.Add(Class);
			// Form class name to search later
			FString ClassName = FString::Printf(TEXT("%s'%s'"), *Class->GetClass()->GetName(), *Class->GetPathName());
			OutAllowedBlueprintClassNames->AddUnique(ClassName);
		}
	}
}

void SPaperZDAnimNotifyPanel::BindCommands()
{
	check(!UICommandList.IsValid());

	UICommandList = MakeShareable(new FUICommandList);
	const FPaperZDAnimNotifyPanelCommands& Commands = FPaperZDAnimNotifyPanelCommands::Get();

	UICommandList->MapAction(
		Commands.DeleteNotify,
		FExecuteAction::CreateSP(this, &SPaperZDAnimNotifyPanel::OnDeletePressed));
}

FReply SPaperZDAnimNotifyPanel::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (UICommandList->ProcessCommandBindings(InKeyEvent))
	{
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

void SPaperZDAnimNotifyPanel::DeselectAllNotifies()
{
	for (TSharedPtr<SPaperZDAnimNotifyTrack> Track : NotifyTracks)
	{
		Track->DeselectAllNodes();
	}
}

void SPaperZDAnimNotifyPanel::UnregisterSelectedCustomNotify(class UPaperZDAnimNotifyCustom* CustomNotify)
{
	//@TODO: We don't support transaction for unregistering, maybe add it later
	/*const FScopedTransaction Transaction(LOCTEXT("UnregisterNotify", "Unregister Notify"));
	AnimBP->Modify();*/
	AnimSequence->GetAnimBP()->UnregisterCustomNotify(CustomNotify->Name); 
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(AnimSequence->GetAnimBP());
}

TArray<UPaperZDAnimNotify_Base*> SPaperZDAnimNotifyPanel::GetAllSelectedNotifyObjects()
{
	TArray<UPaperZDAnimNotify_Base*>SelectedNotifies;
	for (TSharedPtr<SPaperZDAnimNotifyTrack> Track : NotifyTracks)
	{
		SelectedNotifies.Append(Track->GetAllSelectedNotifyObjects());
	}

	return SelectedNotifies;
}

template <class T>
TArray<T*> SPaperZDAnimNotifyPanel::GetAllSelectedNotifyObjects()
{
	TArray<T*>SelectedNotifies;
	for (TSharedPtr<SPaperZDAnimNotifyTrack> Track : NotifyTracks)
	{
		SelectedNotifies.Append(Track->GetAllSelectedNotifyObjects<T>());
	}

	return SelectedNotifies;
}

void SPaperZDAnimNotifyPanel::OnDeletePressed()
{
	// If there's no focus on the panel it's likely the user is not editing notifies
	// so don't delete anything when the key is pressed.
	if (HasKeyboardFocus() || HasFocusedDescendants())
	{
		DeleteSelectedNodes();
	}
}

void SPaperZDAnimNotifyPanel::DeleteSelectedNodes()
{
	const FScopedTransaction Transaction(LOCTEXT("DeleteNotify", "Deleted Notifies"));
	AnimSequence->Modify();

	for (TSharedPtr<SPaperZDAnimNotifyTrack> Track : NotifyTracks)
	{
		for (UPaperZDAnimNotify_Base *Notify : Track->GetAllSelectedNotifyObjects())
		{
			AnimSequence->RemoveNotify(Notify);
		}
	}

	Update();
}

void SPaperZDAnimNotifyPanel::OnGetNotifyBlueprintData(TArray<FAssetData>& OutNotifyData, TArray<FString>* InOutAllowedClassNames)
{
	// If we have nothing to seach with, early out
	if (InOutAllowedClassNames == NULL || InOutAllowedClassNames->Num() == 0)
	{
		return;
	}

	TArray<FAssetData> AssetDataList;
	TArray<FString> FoundClasses;

	// Load the asset registry module
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	// Collect a full list of assets with the specified class
	AssetRegistryModule.Get().GetAssetsByClass(UBlueprint::StaticClass()->GetFName(), AssetDataList);

	static const FName BPParentClassName(TEXT("ParentClass"));
	static const FName BPGenClassName(TEXT("GeneratedClass"));

	int32 BeginClassCount = InOutAllowedClassNames->Num();
	int32 CurrentClassCount = -1;

	while (BeginClassCount != CurrentClassCount)
	{
		BeginClassCount = InOutAllowedClassNames->Num();

		for (int32 AssetIndex = 0; AssetIndex < AssetDataList.Num(); ++AssetIndex)
		{
			FAssetData& AssetData = AssetDataList[AssetIndex];
			FString TagValue = AssetData.GetTagValueRef<FString>(BPParentClassName);

			if (InOutAllowedClassNames->Contains(TagValue))
			{
				FString GenClass = AssetData.GetTagValueRef<FString>(BPGenClassName);

				if (!OutNotifyData.Contains(AssetData))
				{
					// Output the assetdata and record it as found in this request
					OutNotifyData.Add(AssetData);
					FoundClasses.Add(GenClass);
				}

				if (!InOutAllowedClassNames->Contains(GenClass))
				{
					// Expand the class list to account for a new possible parent class found
					InOutAllowedClassNames->Add(GenClass);
				}
			}
		}

		CurrentClassCount = InOutAllowedClassNames->Num();
	}

	// Count native classes, so we don't remove them from the list
	int32 NumNativeClasses = 0;
	for (FString& AllowedClass : *InOutAllowedClassNames)
	{
		if (!AllowedClass.EndsWith(FString(TEXT("_C'"))))
		{
			++NumNativeClasses;
		}
	}

	if (FoundClasses.Num() < InOutAllowedClassNames->Num() - NumNativeClasses)
	{
		// Less classes found, some may have been deleted or reparented
		for (int32 ClassIndex = InOutAllowedClassNames->Num() - 1; ClassIndex >= 0; --ClassIndex)
		{
			FString& ClassName = (*InOutAllowedClassNames)[ClassIndex];
			if (ClassName.EndsWith(FString(TEXT("_C'"))) && !FoundClasses.Contains(ClassName))
			{
				InOutAllowedClassNames->RemoveAt(ClassIndex);
			}
		}
	}
}

FReply SPaperZDAnimNotifyPanel::OnNotifyNodesDragStarted(const FVector2D& ScreenCursorPos, const bool bDragOnMarker)
{
	TSharedRef<SOverlay> NodeDragDecorator = SNew(SOverlay);
	TArray<TSharedPtr<SPaperZDAnimNotifyNode>> Nodes;

	for (TSharedPtr<SPaperZDAnimNotifyTrack> Track : NotifyTracks)
	{
		Track->DisconnectSelectedNodesForDrag(Nodes);
	}

	FVector2D OverlayOrigin = Nodes[0]->GetScreenPosition();
	FVector2D OverlayExtents = OverlayOrigin;
	OverlayExtents.X += Nodes[0]->GetDurationSize();
	for (int32 Idx = 1; Idx < Nodes.Num(); ++Idx)
	{
		TSharedPtr<SPaperZDAnimNotifyNode> Node = Nodes[Idx];
		FVector2D NodePosition = Node->GetScreenPosition();
		float NodeDuration = Node->GetDurationSize();

		if (NodePosition.X < OverlayOrigin.X)
		{
			OverlayOrigin.X = NodePosition.X;
		}
		else if (NodePosition.X + NodeDuration > OverlayExtents.X)
		{
			OverlayExtents.X = NodePosition.X + NodeDuration;
		}

		if (NodePosition.Y < OverlayOrigin.Y)
		{
			OverlayOrigin.Y = NodePosition.Y;
		}
		else if (NodePosition.Y + NotifyHeight > OverlayExtents.Y)
		{
			OverlayExtents.Y = NodePosition.Y + NotifyHeight;
		}
	}
	OverlayExtents -= OverlayOrigin;
	TArray<FVector2D> OffsetInfo;
	for ( int i = 0; i < Nodes.Num(); i++ )
	{
		TSharedPtr<SPaperZDAnimNotifyNode> Node = Nodes[i];
		FVector2D OffsetFromFirst(Node->GetScreenPosition() - OverlayOrigin);

		NodeDragDecorator->AddSlot()
			.Padding(FMargin(OffsetFromFirst.X, OffsetFromFirst.Y, 0.0f, 0.0f))
			[
				Node->AsShared()
			];

		OffsetInfo.Add(OffsetFromFirst);
	}

	FOnDropOperationComplete UpdateDelegate = FOnDropOperationComplete::CreateSP(this, &SPaperZDAnimNotifyPanel::Update);
	return FReply::Handled().BeginDragDrop(FNotifyDragDropOperation::Create(Nodes, NodeDragDecorator, UpdateDelegate, OverlayOrigin - ScreenCursorPos, OverlayExtents, NotifyTracks, OffsetInfo));
}

#undef LOCTEXT_NAMESPACE
