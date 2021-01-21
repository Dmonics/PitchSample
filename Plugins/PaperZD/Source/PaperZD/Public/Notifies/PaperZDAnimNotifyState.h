// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once

#include "Notifies/PaperZDAnimNotify_Base.h"
#include "PaperZDAnimNotifyState.generated.h"

UCLASS(Blueprintable, Abstract)
class PAPERZD_API UPaperZDAnimNotifyState : public UPaperZDAnimNotify_Base
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "PaperZD")
	float Duration;

	//Called each Tick to process the notify and trigger it when necessary
	virtual void TickNotify(float DeltaTime, float Playtime, float LastPlaybackTime, class UPrimitiveComponent* AnimRenderComponent, UPaperZDAnimInstance* OwningInstance = nullptr) override;

	UFUNCTION(BlueprintNativeEvent, Category="PaperZD")
	void OnNotifyBegin(UPaperZDAnimInstance *OwningInstance);

	UFUNCTION(BlueprintNativeEvent, Category = "PaperZD")
	void OnNotifyTick(float DeltaTime, UPaperZDAnimInstance *OwningInstance);

	UFUNCTION(BlueprintNativeEvent, Category = "PaperZD")
	void OnNotifyEnd(UPaperZDAnimInstance *OwningInstance);

private:
	bool bWasActive;
};
