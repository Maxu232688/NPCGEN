#include "NPCPlacementWidget.h"
#include "NPCCoordinator.h"
#include "NPCListWidget.h"
#include "NPCPropertyPanel.h"
#include "NPCConfigDataAsset.h"
#include "PlacedNPCActor.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Styling/SlateTypes.h"

void SNPCPlacementWidget::Construct(const FArguments& InArgs)
{
	CachedConfig = InArgs._NPCConfig;
	CachedCoordinator = InArgs._Coordinator;

	UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] PlacementWidget::Construct - Config=%s (definitions=%d), Coordinator=%s"),
		CachedConfig ? *CachedConfig->GetName() : TEXT("null"),
		CachedConfig ? CachedConfig->NPCDefinitions.Num() : 0,
		CachedCoordinator ? TEXT("valid") : TEXT("null"));

	ChildSlot
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(10.0f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(8.0f)
			[
				SAssignNew(NPCList, SNPCListWidget)
				.NPCConfig(CachedConfig)
				.OnNPCTypeSelected_Lambda([this](int32 Index)
				{
					if (CachedCoordinator)
					{
						CachedCoordinator->SelectNPCType(Index);
					}
				})
			]
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(10.0f)
		[
			SAssignNew(PropertyPanel, SNPCPropertyPanel)
			.TargetNPC(nullptr)
		]
	];

	UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] PlacementWidget::Construct END"));
}

void SNPCPlacementWidget::RefreshNPCList(const TArray<APlacedNPCActor*>& PlacedNPCs)
{
	if (NPCList.IsValid())
		NPCList->RefreshList();
}

void SNPCPlacementWidget::SetSelectedNPCType(int32 Index)
{
	if (NPCList.IsValid())
		NPCList->SetSelectedIndex(Index);
}

void SNPCPlacementWidget::SelectNPC(APlacedNPCActor* NPC)
{
	UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] PlacementWidget::SelectNPC - NPC=%s"),
		NPC ? *NPC->GetName() : TEXT("null"));

	if (!PropertyPanel.IsValid() || !NPC) return;

	if (CachedCoordinator)
		CachedCoordinator->SetSelectedNPC(NPC);

	PropertyPanel->SetTargetNPC(NPC);

	USkeletalMeshComponent* MeshComp = NPC->GetMeshComponent();
	if (MeshComp)
	{
		USkeletalMesh* SkelMesh = MeshComp->GetSkeletalMeshAsset();
		if (SkelMesh)
		{
			FNPCDefinition Def;
			TArray<TSoftObjectPtr<UAnimSequence>> PreconfiguredAnims;
			TArray<TSubclassOf<UAnimInstance>> PreconfiguredABPs;
			if (CachedConfig && CachedConfig->GetNPCConfig(NPC->NPCIndex, Def))
			{
				PreconfiguredAnims = Def.AvailableAnimations;
				PreconfiguredABPs = Def.AvailableAnimBlueprints;
			}

			TArray<UClass*> ABPs = APlacedNPCActor::DiscoverAnimBlueprintsForMesh(SkelMesh, PreconfiguredABPs);
			PropertyPanel->SetAvailableAnimBlueprints(ABPs);
			UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] PlacementWidget::SelectNPC - Set %d available ABPs"), ABPs.Num());

			TArray<UAnimSequence*> Anims = APlacedNPCActor::DiscoverAnimationsForMesh(SkelMesh, PreconfiguredAnims);
			PropertyPanel->SetAvailableAnimations(Anims);
			UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] PlacementWidget::SelectNPC - Set %d available raw anims"), Anims.Num());
		}
	}
}

void SNPCPlacementWidget::ClearNPCPanel()
{
	if (PropertyPanel.IsValid())
		PropertyPanel->ClearTarget();
}
