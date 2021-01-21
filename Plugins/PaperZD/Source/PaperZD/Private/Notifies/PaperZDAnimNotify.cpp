// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "Notifies/PaperZDAnimNotify.h"
#include "PaperZD.h"

UPaperZDAnimNotify::UPaperZDAnimNotify(const FObjectInitializer& ObjectInitializer)
	: Super()
{
}

void UPaperZDAnimNotify::TickNotify(float DeltaTime, float Playtime, float LastPlaybackTime, UPrimitiveComponent* AnimRenderComponent, UPaperZDAnimInstance* OwningInstance /* = nullptr*/)
{
	//Super takes care of setting world context object
	Super::TickNotify(DeltaTime, Playtime, LastPlaybackTime, AnimRenderComponent, OwningInstance);

	//Normal Notifies tick only once, look for the time
	if (Playtime > Time && LastPlaybackTime <= Time)
	{
		OnReceiveNotify(OwningInstance);
	}
}

void UPaperZDAnimNotify::OnReceiveNotify_Implementation(UPaperZDAnimInstance *OwningInstance /* = nullptr*/)
{
	//Empty implementation
}
