#include "NPCPlacementToolEditorModule.h"
#include "NPCPlacementToolbarButton.h"

#define LOCTEXT_NAMESPACE "NPCPlacementToolEditorModule"

void FNPCPlacementToolEditorModule::StartupModule()
{
	ToolbarButton = MakeShareable(new FNPCPlacementToolbarButton);
	ToolbarButton->Initialize();
}

void FNPCPlacementToolEditorModule::ShutdownModule()
{
	if (ToolbarButton.IsValid())
	{
		ToolbarButton->Shutdown();
		ToolbarButton.Reset();
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FNPCPlacementToolEditorModule, NPCPlacementToolEditor)
