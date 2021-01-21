// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once
#include "PaperZDAnimInstance.h"
#include "Components/PrimitiveComponent.h"
#include "PaperZDAnimNotify_Base.generated.h"

/**
 * Base class for all the plugin's notifies.
 */
UCLASS(abstract, hidecategories = UObject, collapsecategories)
class PAPERZD_API UPaperZDAnimNotify_Base : public UObject
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, Category = "AnimNotify", meta = (UIMin = "0.0", ClampMin = "0.0"))
	float Time;

	UPROPERTY()
	FName Name;

	UPROPERTY(EditAnywhere, Category = "AnimNotify")
	FLinearColor Color;

	UPROPERTY()
	int TrackIndex;

	/* The render component currently used by the AnimSequence that owns this notify. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "AnimNotify")
	UPrimitiveComponent* SequenceRenderComponent;

public:
	/**
	* We need the GetWorld defined, if not, the collision or trace functions won't be available in blueprints.
	* For this to work, the world must be correctly defined when TickNotify does get called, the world can be obtained from RenderComponent used for the AnimSequence
	*/
	virtual class UWorld* GetWorld() const override;

	//Called each Tick to process the notify and trigger it when necessary 
	//Playtime and PreviousPlaytime are given for convenience, because the animation can loop
	virtual void TickNotify(float DeltaTime, float Playtime, float LastPlaybackTime, UPrimitiveComponent* AnimRenderComponent, UPaperZDAnimInstance* OwningInstance = nullptr);

	/**
	 * Obtain the name to be displayed on the editor's detail's panel
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Notify")
	FName GetDisplayName() const;

protected:
	/* Obtain the asset that contains this notify instance.*/
	UObject* GetContainingAsset() const;
};
