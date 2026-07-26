#include "NPCPropertyPanel.h"
#include "PlacedNPCActor.h"
#include "NPCPlacementCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Styling/SlateTypes.h"

void SNPCPropertyPanel::Construct(const FArguments& InArgs)
{
	CachedNPC = InArgs._TargetNPC;
	BuildUI();
}

void SNPCPropertyPanel::SetTargetNPC(APlacedNPCActor* NPC)
{
	CachedNPC = NPC;
	UpdateDisplay();
}

void SNPCPropertyPanel::ClearTarget()
{
	CachedNPC = nullptr;
	AvailableABPs.Empty();
	AvailableAnims.Empty();
	if (ABPListBox.IsValid()) ABPListBox->ClearChildren();
	if (AnimListBox.IsValid()) AnimListBox->ClearChildren();
	UpdateDisplay();
}

void SNPCPropertyPanel::SetAvailableAnimBlueprints(const TArray<UClass*>& ABPClasses)
{
	AvailableABPs = ABPClasses;
	RebuildABPList();
}

void SNPCPropertyPanel::SetAvailableAnimations(const TArray<UAnimSequence*>& Animations)
{
	AvailableAnims = Animations;
	RebuildAnimList();
}

void SNPCPropertyPanel::UpdateDisplay()
{
	if (!CachedNPC)
	{
		if (NameText.IsValid()) NameText->SetText(FText::FromString(TEXT("No NPC Selected")));
		if (PositionText.IsValid()) PositionText->SetText(FText::FromString(TEXT("")));
		if (RotationText.IsValid()) RotationText->SetText(FText::FromString(TEXT("")));
		if (ScaleText.IsValid()) ScaleText->SetText(FText::FromString(TEXT("")));
		if (AnimText.IsValid()) AnimText->SetText(FText::FromString(TEXT("")));
		return;
	}

	FVector Loc = CachedNPC->GetActorLocation();
	FRotator Rot = CachedNPC->GetActorRotation();
	FVector Scale = CachedNPC->GetActorScale3D();

	if (NameText.IsValid()) NameText->SetText(FText::FromString(CachedNPC->GetName()));
	if (PositionText.IsValid()) PositionText->SetText(FText::FromString(FString::Printf(TEXT("Pos: %.1f, %.1f, %.1f"), Loc.X, Loc.Y, Loc.Z)));
	if (RotationText.IsValid()) RotationText->SetText(FText::FromString(FString::Printf(TEXT("Rot: %.1f, %.1f, %.1f"), Rot.Pitch, Rot.Yaw, Rot.Roll)));
	if (ScaleText.IsValid()) ScaleText->SetText(FText::FromString(FString::Printf(TEXT("Scale: %.2f, %.2f, %.2f"), Scale.X, Scale.Y, Scale.Z)));

	TSubclassOf<UAnimInstance> ABPClass = CachedNPC->GetAnimBlueprintClass();
	if (ABPClass)
	{
		if (AnimText.IsValid()) AnimText->SetText(FText::FromString(FString::Printf(TEXT("ABP: %s"), *ABPClass->GetName())));
	}
	else
	{
		UAnimSequence* Anim = CachedNPC->GetCurrentAnimation();
		if (AnimText.IsValid()) AnimText->SetText(FText::FromString(FString::Printf(TEXT("Raw Anim: %s"), Anim ? *Anim->GetName() : TEXT("None"))));
	}
}

void SNPCPropertyPanel::SelectAnimBlueprint(UClass* ABPClass)
{
	if (CachedNPC && ABPClass)
	{
		CachedNPC->SetAnimBlueprintClass(ABPClass);
		UpdateDisplay();
		RebuildABPList();

		if (UWorld* World = CachedNPC->GetWorld())
		{
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				if (ANPCPlacementCharacter* Char = Cast<ANPCPlacementCharacter>(PC->GetPawn()))
					Char->SetMouseCapture(true);
			}
		}
	}
}

void SNPCPropertyPanel::SelectAnimation(UAnimSequence* Anim)
{
	if (CachedNPC && Anim)
	{
		CachedNPC->PlayAnimation(Anim, true);
		UpdateDisplay();
		RebuildAnimList();

		if (UWorld* World = CachedNPC->GetWorld())
		{
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				if (ANPCPlacementCharacter* Char = Cast<ANPCPlacementCharacter>(PC->GetPawn()))
					Char->SetMouseCapture(true);
			}
		}
	}
}

void SNPCPropertyPanel::RebuildABPList()
{
	if (!ABPListBox.IsValid()) return;
	ABPListBox->ClearChildren();

	if (AvailableABPs.Num() == 0)
	{
		ABPListBox->AddSlot().AutoHeight().Padding(0, 2)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("No ABPs found")))
			.ColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.5f))
			.Font(FCoreStyle::GetDefaultFontStyle("Italic", 10))
		];
		return;
	}

	TSubclassOf<UAnimInstance> CurrentABP = CachedNPC ? CachedNPC->GetAnimBlueprintClass() : nullptr;

	for (UClass* ABPClass : AvailableABPs)
	{
		if (!ABPClass) continue;

		bool bIsCurrent = (ABPClass == CurrentABP.Get());
		FString ABPName = ABPClass->GetName();
		ABPName.RemoveFromEnd(TEXT("_C"));

		FLinearColor BGColor = bIsCurrent
			? FLinearColor(0.2f, 0.6f, 0.2f, 0.8f)
			: FLinearColor(0.08f, 0.08f, 0.08f, 0.6f);

		ABPListBox->AddSlot().AutoHeight().Padding(0, 1)
		[
			SNew(SButton)
			.ButtonStyle(FAppStyle::Get(), "FlatButton")
			.ForegroundColor(FLinearColor::White)
			.OnClicked_Lambda([this, ABPClass]() { SelectAnimBlueprint(ABPClass); return FReply::Handled(); })
			.ContentPadding(FMargin(4.0f, 2.0f))
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				.BorderBackgroundColor(BGColor)
				.Padding(4)
				[
					SNew(STextBlock)
					.Text(FText::FromString(ABPName))
					.Font(FCoreStyle::GetDefaultFontStyle(bIsCurrent ? "Bold" : "Regular", 10))
					.ColorAndOpacity(bIsCurrent ? FLinearColor::White : FLinearColor(0.7f, 0.7f, 0.7f))
				]
			]
		];
	}
}

void SNPCPropertyPanel::RebuildAnimList()
{
	if (!AnimListBox.IsValid()) return;
	AnimListBox->ClearChildren();

	if (AvailableAnims.Num() == 0)
	{
		AnimListBox->AddSlot().AutoHeight().Padding(0, 2)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("No raw anims found")))
			.ColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.5f))
			.Font(FCoreStyle::GetDefaultFontStyle("Italic", 10))
		];
		return;
	}

	UAnimSequence* CurrentAnim = CachedNPC ? CachedNPC->GetCurrentAnimation() : nullptr;

	for (UAnimSequence* Anim : AvailableAnims)
	{
		if (!Anim) continue;

		bool bIsCurrent = (Anim == CurrentAnim);
		FString AnimName = Anim->GetName();
		FLinearColor BGColor = bIsCurrent
			? FLinearColor(0.2f, 0.5f, 0.8f, 0.8f)
			: FLinearColor(0.08f, 0.08f, 0.08f, 0.6f);

		AnimListBox->AddSlot().AutoHeight().Padding(0, 1)
		[
			SNew(SButton)
			.ButtonStyle(FAppStyle::Get(), "FlatButton")
			.ForegroundColor(FLinearColor::White)
			.OnClicked_Lambda([this, Anim]() { SelectAnimation(Anim); return FReply::Handled(); })
			.ContentPadding(FMargin(4.0f, 2.0f))
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				.BorderBackgroundColor(BGColor)
				.Padding(4)
				[
					SNew(STextBlock)
					.Text(FText::FromString(AnimName))
					.Font(FCoreStyle::GetDefaultFontStyle(bIsCurrent ? "Bold" : "Regular", 10))
					.ColorAndOpacity(bIsCurrent ? FLinearColor::White : FLinearColor(0.7f, 0.7f, 0.7f))
				]
			]
		];
	}
}

void SNPCPropertyPanel::BuildUI()
{
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(8.0f)
		.Visibility_Lambda([this]() { return CachedNPC ? EVisibility::Visible : EVisibility::Collapsed; })
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
			[
				SAssignNew(NameText, STextBlock)
				.Text(FText::FromString(TEXT("NPC Props")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
			[
				SAssignNew(PositionText, STextBlock)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
			[
				SAssignNew(RotationText, STextBlock)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
			[
				SAssignNew(ScaleText, STextBlock)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
			[
				SAssignNew(AnimText, STextBlock)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 4)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("--- Anim Blueprints ---")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
				.ColorAndOpacity(FLinearColor(0.3f, 0.8f, 0.3f))
			]

			+ SVerticalBox::Slot().MaxHeight(200.0f)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SAssignNew(ABPListBox, SVerticalBox)
				]
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 4)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("--- Raw Animations ---")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
				.ColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.3f))
			]

			+ SVerticalBox::Slot().MaxHeight(200.0f)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SAssignNew(AnimListBox, SVerticalBox)
				]
			]
		]
	];
}
