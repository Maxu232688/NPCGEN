#include "DiffPreviewWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Styling/SlateTypes.h"

void SDiffPreviewWidget::Construct(const FArguments& InArgs)
{
	OnConfirmedDelegate = InArgs._OnConfirmed;
	OnCancelledDelegate = InArgs._OnCancelled;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(12.0f)
		[
			SNew(SVerticalBox)

			// Title
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("场景差异对比")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
			]

			// Summary
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
			[
				SAssignNew(SummaryText, STextBlock)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
			]

			// Diff list
			+ SVerticalBox::Slot().FillHeight(1.0f).Padding(0, 0, 0, 8)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
				.Padding(4)
				[
					SAssignNew(DiffListBox, SVerticalBox)
				]
			]

			// Buttons
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("确认应用")))
					.OnClicked_Lambda([this]() -> FReply
					{
						OnConfirmedDelegate.ExecuteIfBound();
						return FReply::Handled();
					})
				]

				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("取消")))
					.OnClicked_Lambda([this]() -> FReply
					{
						OnCancelledDelegate.ExecuteIfBound();
						return FReply::Handled();
					})
				]
			]
		]
	];
}

void SDiffPreviewWidget::SetDiffs(const TArray<FActorDiff>& InDiffs)
{
	Diffs = InDiffs;
	RebuildList();
}

void SDiffPreviewWidget::RebuildList()
{
	if (!DiffListBox.IsValid()) return;
	DiffListBox->ClearChildren();

	int32 AddedCount = 0, ModifiedCount = 0, DeletedCount = 0;
	for (const FActorDiff& Diff : Diffs)
	{
		switch (Diff.DiffType)
		{
		case EDiffType::Added: AddedCount++; break;
		case EDiffType::Modified: ModifiedCount++; break;
		case EDiffType::Deleted: DeletedCount++; break;
		default: break;
		}
	}

	if (SummaryText.IsValid())
	{
		SummaryText->SetText(FText::FromString(
			FString::Printf(TEXT("新增: %d  修改: %d  删除: %d"), AddedCount, ModifiedCount, DeletedCount)));
	}

	for (const FActorDiff& Diff : Diffs)
	{
		FLinearColor Color;
		switch (Diff.DiffType)
		{
		case EDiffType::Added: Color = FLinearColor::Green; break;
		case EDiffType::Modified: Color = FLinearColor::Yellow; break;
		case EDiffType::Deleted: Color = FLinearColor::Red; break;
		default: Color = FLinearColor::White; break;
		}

		DiffListBox->AddSlot()
		.AutoHeight()
		.Padding(0, 2)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
			[
				SNew(STextBlock)
				.Text(GetDiffTypeText(Diff.DiffType))
				.ColorAndOpacity(Color)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
			]

			+ SHorizontalBox::Slot().FillWidth(1.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Diff.ActorName))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
			]
		];
	}
}

FText SDiffPreviewWidget::GetDiffTypeText(EDiffType Type) const
{
	switch (Type)
	{
	case EDiffType::Added: return FText::FromString(TEXT("[新增]"));
	case EDiffType::Modified: return FText::FromString(TEXT("[修改]"));
	case EDiffType::Deleted: return FText::FromString(TEXT("[删除]"));
	default: return FText::FromString(TEXT("[未知]"));
	}
}
