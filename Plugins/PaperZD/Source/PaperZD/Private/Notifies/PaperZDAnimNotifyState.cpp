// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "Notifies/PaperZDAnimNotifyState.h"
#include "PaperZD.h"

UPaperZDAnimNotifyState::UPaperZDAnimNotifyState(const FObjectInitializer& ObjectInitializer)
	: Super()
{
	Duration = 0.5f;
	bWasActive = false;
}

void UPaperZDAnimNotifyState::TickNotify(float DeltaTime, float Playtime, float LastPlaybackTime, UPrimitiveComponent* AnimRenderComponent, UPaperZDAnimInstance* OwningInstance /* = nullptr*/)
{
	//Super takes care of setting world context object
	Super::TickNotify(DeltaTime, Playtime, LastPlaybackTime, AnimRenderComponent, OwningInstance);

	//Check for init conditions
	if (Playtime > Time && LastPlaybackTime <= Time) 
	{
		OnNotifyBegin(OwningInstance);
		bWasActive = true;
	}
	else if (Playtime < Time || Playtime > (Time + Duration)) 
	{ 
		// End conditions
		if (bWasActive) 
		{
			OnNotifyEnd(OwningInstance);
			bWasActive = false;
		}

		return;
	}
	else if(bWasActive)
	{ 
		//Tick conditions, validate that Begin was done first
		OnNotifyTick(DeltaTime, OwningInstance);
	}
}

void UPaperZDAnimNotifyState::OnNotifyBegin_Implementation(UPaperZDAnimInstance *OwningInstance /* = nullptr*/)
{
	//Empty Implementation
}

void UPaperZDAnimNotifyState::OnNotifyTick_Implementation(float DeltaTime, UPaperZDAnimInstance *OwningInstance /* = nullptr*/)
{
	//Empty Implementation
}

void UPaperZDAnimNotifyState::OnNotifyEnd_Implementation(UPaperZDAnimInstance *OwningInstance /* = nullptr*/)
{
	//Empty Implementation
}
