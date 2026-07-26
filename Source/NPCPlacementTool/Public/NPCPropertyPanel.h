#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Animation/AnimSequence.h"

class APlacedNPCActor;
class UAnimInstance;
class SVerticalBox;

class NPCPLACEMENTTOOL_API SNPCPropertyPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNPCPropertyPanel)
		: _TargetNPC(nullptr)
	{}
		SLATE_ARGUMENT(APlacedNPCActor*, TargetNPC)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void SetTargetNPC(APlacedNPCActor* NPC);
	void ClearTarget();
	void UpdateDisplay();

	void SetAvailableAnimBlueprints(const TArray<UClass*>& ABPClasses);

	void SetAvailableAnimations(const TArray<UAnimSequence*>& Animations);

private:
	APlacedNPCActor* CachedNPC = nullptr;

	TSharedPtr<STextBlock> NameText;
	TSharedPtr<STextBlock> PositionText;
	TSharedPtr<STextBlock> RotationText;
	TSharedPtr<STextBlock> ScaleText;
	TSharedPtr<STextBlock> AnimText;

	TSharedPtr<SVerticalBox> ABPListBox;
	TSharedPtr<SVerticalBox> AnimListBox;

	TArray<UClass*> AvailableABPs;
	TArray<UAnimSequence*> AvailableAnims;

	void BuildUI();
	void RebuildABPList();
	void RebuildAnimList();
	void SelectAnimBlueprint(UClass* ABPClass);
	void SelectAnimation(UAnimSequence* Anim);
};
