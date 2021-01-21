// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once
#include "Notifies/PaperZDAnimNotify.h"
#include "PaperZDAnimNotify_PlaySound.generated.h"

class USoundBase;
UCLASS(Blueprintable, meta=(DisplayName="Play Sound"))
class PAPERZD_API UPaperZDAnimNotify_PlaySound : public UPaperZDAnimNotify
{
	GENERATED_UCLASS_BODY()

public:
	// Sound to Play
	UPROPERTY(EditAnywhere, Category = "AnimNotify")
	USoundBase* Sound;

	// Volume Multiplier
	UPROPERTY(EditAnywhere, Category = "AnimNotify")
		float VolumeMultiplier;

	// Pitch Multiplier
	UPROPERTY(EditAnywhere, Category = "AnimNotify")
		float PitchMultiplier;

	// If this sound should follow its owner
	UPROPERTY(EditAnywhere, Category = "AnimNotify")
		uint32 bFollow : 1;

	// Socket or bone name to attach sound to
	UPROPERTY(EditAnywhere, Category = "AnimNotify", meta = (EditCondition = "bFollow"))
	FName AttachName;

public:
	void OnReceiveNotify_Implementation(UPaperZDAnimInstance *OwningInstance = nullptr) override;
	FName GetDisplayName_Implementation() const override;

};
