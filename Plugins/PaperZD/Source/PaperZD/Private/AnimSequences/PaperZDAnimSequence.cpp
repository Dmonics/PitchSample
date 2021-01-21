// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "AnimSequences/PaperZDAnimSequence.h"
#include "PaperZD.h"
#include "PaperZDAnimInstance.h"
#include "Components/PrimitiveComponent.h"

//Notifies
#include "Notifies/PaperZDAnimNotify.h"
#include "Notifies/PaperZDAnimNotifyState.h"
#include "Notifies/PaperZDAnimNotifyCustom.h"

/************************************************************************/
/* Track                                                                */
/************************************************************************/
UPaperZDAnimTrack::UPaperZDAnimTrack(const FObjectInitializer& ObjectInitializer)
	: Super(), AnimNotifies(this)
{
}

/************************************************************************/
/* Sequence                                                             */
/************************************************************************/
//Setup static variables
const FName UPaperZDAnimSequence::DefaultCategory(TEXT("Default"));
const FName UPaperZDAnimSequence::AnimBPMemberName(TEXT("AnimBP"));

UPaperZDAnimSequence::UPaperZDAnimSequence(const FObjectInitializer& ObjectInitializer)
	: Super(), AnimTracks(this)
{
#if WITH_EDITOR
	SetFlags(GetFlags() | RF_Transactional);
#endif
	Category = UPaperZDAnimSequence::DefaultCategory;
}

void UPaperZDAnimSequence::SetAnimBP(UPaperZDAnimBP *InAnimBP)
{
	AnimBP = InAnimBP;
}

TArray<UPaperZDAnimNotify_Base*> UPaperZDAnimSequence::GetAnimNotifies() const
{
	return AnimNotifies;
}

FName UPaperZDAnimSequence::GetSequenceName() const
{
	return GetFName();
}

float UPaperZDAnimSequence::GetTotalDuration() const
{
	return 0.0f;
}

float UPaperZDAnimSequence::GetFramesPerSecond() const
{
	return 24.0f;
}

TSubclassOf<UPrimitiveComponent> UPaperZDAnimSequence::GetRenderComponentClass() const
{
	return UPrimitiveComponent::StaticClass();
}

#if WITH_EDITOR
void UPaperZDAnimSequence::PostEditUndo()
{
	Super::PostEditUndo();
	UpdateCachedInfo();

	OnPostEditUndo.ExecuteIfBound();
}

void UPaperZDAnimSequence::PostLoad()
{
	Super::PostLoad();
	InitTracks();

}

void UPaperZDAnimSequence::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);

	//Save the non copied versions
	TArray<UPaperZDAnimTrack *> Tracks(CachedAnimTracks);
	TArray<UPaperZDAnimNotify_Base *> Notifies(AnimNotifies);

	//Duplicate
	CachedAnimTracks.Empty();
	for (UPaperZDAnimTrack *Track : Tracks)
	{
		CachedAnimTracks.Add(DuplicateObject(Track, GetOuter()));
	}

	AnimNotifies.Empty();
	for (UPaperZDAnimNotify_Base *Notify : Notifies)
	{
		AnimNotifies.Add(DuplicateObject(Notify, GetOuter()));
	}

}

void UPaperZDAnimSequence::PostRename(UObject* OldOuter, const FName OldName)
{
	Super::PostRename(OldOuter, OldName);

	//Rename
	for (UPaperZDAnimTrack *Track : CachedAnimTracks)
	{
		Track->Rename(*Track->GetFName().ToString(), GetOuter());
	}

	for (UPaperZDAnimNotify_Base *Notify : AnimNotifies)
	{
		Notify->Rename(*Notify->GetFName().ToString(), GetOuter());
	}

}

int32 UPaperZDAnimSequence::CreateTrack(int32 InsertInto)
{
	UPaperZDAnimTrack *TrackToInsert = NewObject<UPaperZDAnimTrack>(GetOuter(), UPaperZDAnimTrack::StaticClass(), NAME_None, RF_Transactional);

	//@NOTE: Fix for TTransArray not working with Insert Method
	TArray<UPaperZDAnimTrack*> TempTrackArray = AnimTracks;
	AnimTracks.Empty();
	bool bInserted = false;
	int32 index = INDEX_NONE;
	for (int32 i = 0; i < TempTrackArray.Num(); i++)
	{
		if (i == InsertInto) {
			index = AnimTracks.Add(TrackToInsert);
			bInserted = true;
		}

		AnimTracks.Add(TempTrackArray[i]);
	}

	if (!bInserted)
	{
		index = AnimTracks.Add(TrackToInsert);
	}

	//Just to be sure, check that the index was set
	check(index != INDEX_NONE);

	UpdateCachedInfo();
	return index;

}

void UPaperZDAnimSequence::RemoveTrack(int32 Index)
{
	check(AnimTracks.Num() > Index);
	AnimTracks.RemoveAt(Index);
	UpdateCachedInfo();
}

void UPaperZDAnimSequence::AddNotifyToTrack(UPaperZDAnimNotify_Base* Notify, int32 TrackIndex)
{
	//AnimNotifies.Add(Notify);
	AnimTracks[TrackIndex]->AnimNotifies.Add(Notify);
	UpdateCachedInfo();
}

void UPaperZDAnimSequence::AddNotifyToTrack(TSubclassOf<UPaperZDAnimNotify_Base> NotifyClass, int32 TrackIndex, FName NotifyName, float InitTime)
{
	check(AnimTracks.Num() > TrackIndex);

	UPaperZDAnimNotify_Base *N = NewObject<UPaperZDAnimNotify_Base>(this, NotifyClass, NAME_None, RF_Transactional);
	N->Time = InitTime;
	N->Name = NotifyName;
	AddNotifyToTrack(N, TrackIndex);
}

void UPaperZDAnimSequence::RemoveNotify(UPaperZDAnimNotify_Base* Notify)
{
	int FromTrack = GetNotifyTrack(Notify);
	if (ensure(FromTrack != INDEX_NONE))
	{
		AnimTracks[FromTrack]->AnimNotifies.Remove(Notify);
		UpdateCachedInfo();
	}
}

void UPaperZDAnimSequence::MoveNotify(UPaperZDAnimNotify_Base* Notify, int32 ToTrack)
{
	int FromTrack = GetNotifyTrack(Notify);
	if (ensure(FromTrack != INDEX_NONE) || FromTrack == ToTrack)
	{
		//Only add the notifies to the arrays, every other actions was made on adding notify
		AnimTracks[FromTrack]->AnimNotifies.Remove(Notify);
		AnimTracks[ToTrack]->AnimNotifies.Add(Notify);
		UpdateCachedInfo();
	}
}

int UPaperZDAnimSequence::GetNotifyTrack(UPaperZDAnimNotify_Base* Notify)
{
	for (int i = 0; i < AnimTracks.Num(); i++)
	{
		UPaperZDAnimTrack *Track = AnimTracks[i];
		for (UPaperZDAnimNotify_Base *Noti : Track->AnimNotifies)
		{
			if (Noti == Notify)
			{
				return i;
			}
		}
	}

	return INDEX_NONE;
}

void UPaperZDAnimSequence::InitTracks()
{
	//Recover the tracks
	AnimTracks.Empty();
	for (UPaperZDAnimTrack *Track : CachedAnimTracks)
	{
		if (Track->IsValidLowLevel()) 
		{
			AnimTracks.Add(Track);

			//This is a patch, if the AnimNotifies TTransArray contains a compilable AnimNotify, the notify itself could be invalid due to possible blueprint compilation of the AnimBP
			//In this case triggering Empty() will try to save the object to the GUndo Buffer which will crash the program. By using this we circumvent GUndo's lock
			//Track->AnimNotifies.Empty();
			Track->AnimNotifies = TTransArray<UPaperZDAnimNotify_Base*>(Track);
		}
	}

	//Recover the AnimNotifies
	for (UPaperZDAnimNotify_Base *Notify : AnimNotifies)
	{
		if (Notify->IsValidLowLevel() && ensure(Notify->TrackIndex < AnimTracks.Num())) //Notify could not be valid at low level, deleted a Blueprint Notify, etc
			AnimTracks[Notify->TrackIndex]->AnimNotifies.Add(Notify);
	}

	//If no cached anim track was available, create one
	if (!AnimTracks.Num())
		CreateTrack();
}

void UPaperZDAnimSequence::UpdateCachedInfo()
{
	CachedAnimTracks.Empty();
	AnimNotifies.Empty();

	for (int i = 0; i < AnimTracks.Num(); i++)
	{
		UPaperZDAnimTrack *Track = AnimTracks[i];
		CachedAnimTracks.Add(Track);

		for (UPaperZDAnimNotify_Base *Notify : Track->AnimNotifies)
		{
			if (Notify->IsValidLowLevel()) {
				Notify->TrackIndex = i;
				AnimNotifies.Add(Notify);
			}

		}
	}
}

void UPaperZDAnimSequence::CleanInvalidNodes(TArray<FName>ValidRegisteredNames)
{
	//Make sure we have updated info on the tracks
	InitTracks();

	for (UPaperZDAnimTrack *Track : AnimTracks)
	{
		for (int i = Track->AnimNotifies.Num() - 1; i >= 0; i--)
		{
			UPaperZDAnimNotifyCustom *CustomNotify = Cast<UPaperZDAnimNotifyCustom>(Track->AnimNotifies[i]);

			if (CustomNotify && !ValidRegisteredNames.Contains(CustomNotify->Name))
				Track->AnimNotifies.RemoveAt(i);
		}
	}

	UpdateCachedInfo();
}
#endif
