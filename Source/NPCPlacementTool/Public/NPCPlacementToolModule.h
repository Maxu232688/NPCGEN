#pragma once

#include "Modules/ModuleManager.h"

class FNPCPlacementToolModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
