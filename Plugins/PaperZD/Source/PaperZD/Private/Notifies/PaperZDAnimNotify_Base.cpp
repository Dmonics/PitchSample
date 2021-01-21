// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "Notifies/PaperZDAnimNotify_Base.h"
#include "AnimSequences/PaperZDAnimSequence.h"
#include "PaperZD.h"

UPaperZDAnimNotify_Base::UPaperZDAnimNotify_Base(const FObjectInitializer& ObjectInitializer)
	: Super()
{
	Name = FName(TEXT("BaseNotify"));
	Color = FLinearColor::Red;
}

UWorld* UPaperZDAnimNotify_Base::GetWorld() const
{
	return SequenceRenderComponent ? SequenceRenderComponent->GetWorld() : NULL;
}

FName UPaperZDAnimNotify_Base::GetDisplayName_Implementation() const
{
	return Name;
}

void UPaperZDAnimNotify_Base::TickNotify(float DeltaTime, float Playtime, float LastPlaybackTime, UPrimitiveComponent* AnimRenderComponent, UPaperZDAnimInstance* OwningInstance /* = nullptr */)
{
	SequenceRenderComponent = AnimRenderComponent;
}

UObject* UPaperZDAnimNotify_Base::GetContainingAsset() const
{
	UObject* ContainingAsset = GetTypedOuter<UPaperZDAnimSequence>();
	if (ContainingAsset == nullptr)
	{
		ContainingAsset = GetOutermost();
	}
	return ContainingAsset;
}
