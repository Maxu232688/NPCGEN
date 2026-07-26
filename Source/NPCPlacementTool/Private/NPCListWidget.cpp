#include "NPCListWidget.h"
#include "NPCConfigDataAsset.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Styling/SlateTypes.h"

void SNPCListWidget::Construct(const FArguments& InArgs)
{
	CachedConfig = InArgs._NPCConfig;
	OnNPCTypeSelectedDelegate = InArgs._OnNPCTypeSelected;

	ChildSlot
	[
		SNew(SVerticalBox)

		// Title
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 8)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("NPC 列表")))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
		]

		// List container
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(ListBox, SVerticalBox)
		]
	];

	RebuildList();
}

void SNPCListWidget::SetSelectedIndex(int32 Index)
{
	SelectedIndex = Index;
	RebuildList();
}

void SNPCListWidget::RefreshList()
{
	RebuildList();
}

void SNPCListWidget::RebuildList()
{
	if (!ListBox.IsValid()) return;
	ListBox->ClearChildren();

	if (!CachedConfig) return;

	for (int32 i = 0; i < CachedConfig->NPCDefinitions.Num(); i++)
	{
		const FNPCDefinition& Def = CachedConfig->NPCDefinitions[i];
		bool bIsSelected = (i == SelectedIndex);

		FLinearColor BGColor = bIsSelected ? FLinearColor(0.2f, 0.5f, 0.8f, 0.8f) : FLinearColor(0.1f, 0.1f, 0.1f, 0.6f);

		int32 Index = i;

		ListBox->AddSlot()
		.AutoHeight()
		.Padding(0, 2)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.BorderBackgroundColor(BGColor)
			.Padding(6)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0, 0, 8, 0)
				[
					SNew(STextBlock)
					.Text(FText::FromString(FString::Printf(TEXT("[%d]"), Def.HotKey)))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
					.ColorAndOpacity(FLinearColor::Yellow)
				]

				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Def.DisplayName))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
					.ColorAndOpacity(bIsSelected ? FLinearColor::White : FLinearColor(0.8f, 0.8f, 0.8f))
				]
			]
		];
	}

	// Hint text
	if (SelectedIndex >= 0)
	{
		ListBox->AddSlot()
		.AutoHeight()
		.Padding(0, 8, 0, 0)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("再按一次数字键生成 NPC")))
			.Font(FCoreStyle::GetDefaultFontStyle("Italic", 10))
			.ColorAndOpacity(FLinearColor(0.6f, 0.8f, 1.0f))
		];
	}

	// Empty list guidance
	if (CachedConfig->NPCDefinitions.Num() == 0)
	{
		ListBox->AddSlot()
		.AutoHeight()
		.Padding(0, 8, 0, 0)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("No NPC types configured.\nOpen Content/DefaultNPCs/DA_DefaultNPCConfig\nto add NPC types (meshes, animations).")))
			.Font(FCoreStyle::GetDefaultFontStyle("Italic", 10))
			.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f))
			.AutoWrapText(true)
		];
	}
}
