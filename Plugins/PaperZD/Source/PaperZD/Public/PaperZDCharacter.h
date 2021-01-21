// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once

#include "PaperCharacter.h"
#include "PaperZDAnimInstance.h"
#include "PaperZDCharacter.generated.h"

class UEdGraph;
class UPaperZDAnimInstance;
class UPaperZDAnimBP;
UCLASS()
class PAPERZD_API APaperZDCharacter : public APaperCharacter, public IPaperZDAnimInstanceManager
{
	GENERATED_UCLASS_BODY()

private:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "PaperZD", meta=(AllowPrivateAccess="true"))
	UPaperZDAnimInstance* AnimInstance;

public:
	/* Animation blueprint to use for this character. NULL on packaged projects */
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "PaperZD")
	UPaperZDAnimBP* AnimationBlueprint;

	/* Holds the AnimInstance class, this is used for constructing the AnimInstance, as the AnimBP doesn't cook into a packaged project */
	UPROPERTY()
	UClass* AnimInstanceClass;

#if WITH_EDITOR
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);
	void OnAnimBPCompiled();
#endif

public:
	/* DEPRECATED FUNCTION, changed for AnimInstance and AnimPlayer methods that use AnimSequences. */
	UFUNCTION(BlueprintNativeEvent, Category = "PaperZD", meta=(DeprecatedFunction, DeprecationMessage = "OnAnimationUpdated is deprecated, please use the AnimInstance or AnimPlayer delegates that use AnimSequences instead"))
	void OnAnimationUpdated(class UPaperFlipbook *From, class UPaperFlipbook* To);

	//Creates or gets the Anim Instance
	UPaperZDAnimInstance* GetOrCreateAnimInstance(bool bForceCreation = false);

	//Special AnimInstance used for Sequence playing
	UPaperZDAnimInstance* GetTransientAnimInstance();

	/* Called by Sequencer, allows the character to prepare for a movie sequence, changing the animation update mode so it ignores the Animation Graph. */
	virtual void PrepareForMovieSequence();

	UFUNCTION(BlueprintImplementableEvent, Category = "PaperZD | Movie")
	void OnPrepareForMovieSequence();

	virtual void RestoreFromMovieSequence();

	UFUNCTION(BlueprintImplementableEvent, Category = "PaperZD | Movie")
	void OnRestoreFromMovieSequence();

	//~IPaperZDAnimInstanceManager
	virtual void ConfigurePlayer_Implementation(UPaperZDAnimPlayer* Player) override;
	//~IPaperZDAnimInstanceManager
protected:
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
};
