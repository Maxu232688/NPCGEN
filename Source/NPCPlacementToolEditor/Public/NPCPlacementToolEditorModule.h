#pragma once

#include "Modules/ModuleManager.h"

class FNPCPlacementToolbarButton;

class FNPCPlacementToolEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TSharedPtr<FNPCPlacementToolbarButton> ToolbarButton;
};
