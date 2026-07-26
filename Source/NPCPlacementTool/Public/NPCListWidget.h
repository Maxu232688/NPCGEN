#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class UNPCConfigDataAsset;

DECLARE_DELEGATE_OneParam(FOnNPCTypeSelected, int32);

class NPCPLACEMENTTOOL_API SNPCListWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNPCListWidget)
		: _NPCConfig(nullptr)
	{}
		SLATE_ARGUMENT(UNPCConfigDataAsset*, NPCConfig)
		SLATE_EVENT(FOnNPCTypeSelected, OnNPCTypeSelected)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void SetSelectedIndex(int32 Index);
	void RefreshList();

private:
	UNPCConfigDataAsset* CachedConfig = nullptr;
	FOnNPCTypeSelected OnNPCTypeSelectedDelegate;

	int32 SelectedIndex = -1;

	TSharedPtr<SVerticalBox> ListBox;

	void RebuildList();
};
