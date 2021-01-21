// Copyright 2017-2018 Critical Failure Studio

#pragma once

#include "CoreMinimal.h"
#include "PaperZDAnimSequence.h"
#include "PaperFlipbook.h"
#include "PaperZDAnimSequence_Flipbook.generated.h"

/**
 * 
 */
UCLASS()
class PAPERZD_API UPaperZDAnimSequence_Flipbook : public UPaperZDAnimSequence
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AnimSequence")
	UPaperFlipbook* Flipbook;
	
	//Required methods
	virtual void BeginSequencePlayback(class UPrimitiveComponent* RenderComponent, bool bLooping, bool bIsPreviewPlayback = false) const override;
	virtual void UpdateRenderPlayback(class UPrimitiveComponent* RenderComponent, const float Time, bool bIsPreviewPlayback = false) const override;
	virtual float GetTotalDuration() const override;
	virtual TSubclassOf<UPrimitiveComponent> GetRenderComponentClass() const override;
	virtual void ConfigureRenderComponent(class UPrimitiveComponent* RenderComponent, bool bIsPreviewPlayback = false) const override;
	virtual float GetFramesPerSecond() const override;
	
};
