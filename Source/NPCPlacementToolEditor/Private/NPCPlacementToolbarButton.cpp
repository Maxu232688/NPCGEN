#include "NPCPlacementToolbarButton.h"
#include "NPCPlacementToolSubsystem.h"
#include "ToolMenus.h"
#include "LevelEditor.h"

void FNPCPlacementToolbarButton::Initialize()
{
	CommandList = MakeShareable(new FUICommandList);

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateLambda([this]()
		{
			FLevelEditorModule& LevelEditorModule =
				FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

			TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
			ToolbarExtender->AddToolBarExtension(
				"Play",
				EExtensionHook::After,
				CommandList,
				FToolBarExtensionDelegate::CreateRaw(this, &FNPCPlacementToolbarButton::AddToolbarExtension)
			);

			LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
		})
	);
}

void FNPCPlacementToolbarButton::Shutdown()
{
	UToolMenus::UnregisterOwner(this);
}

void FNPCPlacementToolbarButton::OnStartPlacementClicked()
{
	UNPCPlacementToolSubsystem* Subsystem = GEditor->GetEditorSubsystem<UNPCPlacementToolSubsystem>();
	if (Subsystem)
	{
		if (Subsystem->IsInPlacementMode())
		{
			Subsystem->StopPlacementMode();
		}
		else
		{
			Subsystem->StartPlacementMode();
		}
	}
}

void FNPCPlacementToolbarButton::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(
		FExecuteAction::CreateRaw(this, &FNPCPlacementToolbarButton::OnStartPlacementClicked),
		NAME_None,
		FText::FromString(TEXT("NPC Placement")),
		FText::FromString(TEXT("Start/Stop NPC Placement Tool")),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.GameSettings")
	);
}
