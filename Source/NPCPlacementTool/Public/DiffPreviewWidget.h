#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "DiffDataTypes.h"

DECLARE_DELEGATE(FOnDiffConfirmed);
DECLARE_DELEGATE(FOnDiffCancelled);

class NPCPLACEMENTTOOL_API SDiffPreviewWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDiffPreviewWidget)
	{}
		SLATE_EVENT(FOnDiffConfirmed, OnConfirmed)
		SLATE_EVENT(FOnDiffCancelled, OnCancelled)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void SetDiffs(const TArray<FActorDiff>& InDiffs);

private:
	FOnDiffConfirmed OnConfirmedDelegate;
	FOnDiffCancelled OnCancelledDelegate;

	TSharedPtr<SVerticalBox> DiffListBox;
	TSharedPtr<STextBlock> SummaryText;

	TArray<FActorDiff> Diffs;

	void RebuildList();
	FText GetDiffTypeText(EDiffType Type) const;
};
