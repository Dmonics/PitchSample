// Copyright 2017 Carlos Ibanez Ch. All Rights Reserved.

#pragma once
#include "Modules/ModuleManager.h"

class FPaperZDModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
