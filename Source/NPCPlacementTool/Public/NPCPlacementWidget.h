#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class UNPCConfigDataAsset;
class ANPCCoordinator;
class APlacedNPCActor;
class SNPCListWidget;
class SNPCPropertyPanel;

class NPCPLACEMENTTOOL_API SNPCPlacementWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNPCPlacementWidget)
		: _NPCConfig(nullptr)
		, _Coordinator(nullptr)
	{}
		SLATE_ARGUMENT(UNPCConfigDataAsset*, NPCConfig)
		SLATE_ARGUMENT(ANPCCoordinator*, Coordinator)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void RefreshNPCList(const TArray<APlacedNPCActor*>& PlacedNPCs);
	void SetSelectedNPCType(int32 Index);

	void SelectNPC(APlacedNPCActor* NPC);

	void ClearNPCPanel();

private:
	TSharedPtr<SNPCListWidget> NPCList;
	TSharedPtr<SNPCPropertyPanel> PropertyPanel;

	UNPCConfigDataAsset* CachedConfig = nullptr;
	ANPCCoordinator* CachedCoordinator = nullptr;
};
