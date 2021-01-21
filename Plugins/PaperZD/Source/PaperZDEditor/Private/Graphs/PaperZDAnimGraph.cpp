// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#include "PaperZDAnimGraph.h"
#include "PaperZDAnimBP.h"

//////////////////////////////////////////////////////////////////////////
//// PaperZD Anim Graph
//////////////////////////////////////////////////////////////////////////
UPaperZDAnimGraph::UPaperZDAnimGraph(const FObjectInitializer& ObjectInitializer)
	: Super()
{
}


UPaperZDAnimBP* UPaperZDAnimGraph::GetAnimBP() const
{
	return CastChecked<UPaperZDAnimBP>(GetOuter());
}