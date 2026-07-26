#include "NPCPlacementToolSubsystem.h"
#include "SceneSnapshot.h"
#include "SceneDiffCalculator.h"
#include "NPCConfigDataAsset.h"
#include "NPCCoordinator.h"
#include "PlacedNPCActor.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Editor.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/MessageDialog.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "HAL/FileManager.h"
#include "Misc/PackageName.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

void UNPCPlacementToolSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UNPCPlacementToolSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

UNPCConfigDataAsset* UNPCPlacementToolSubsystem::EnsureDefaultConfigExists()
{
	static const TCHAR* ConfigPaths[] = {
		TEXT("/Game/DefaultNPCs/DA_DefaultNPCConfig"),
		TEXT("/Game/DA_DefaultNPCConfig"),
		TEXT("/NPCPlacementTool/DA_DefaultNPCConfig"),
	};
	for (const TCHAR* ConfigPath : ConfigPaths)
	{
		UNPCConfigDataAsset* Found = LoadObject<UNPCConfigDataAsset>(nullptr, ConfigPath);
		if (Found)
		{
			UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] Found existing config: %s"), ConfigPath);
			return Found;
		}
	}

	const FString PackagePath = TEXT("/Game/DefaultNPCs/");
	const FString AssetName = TEXT("DA_DefaultNPCConfig");

	const FString FullDir = FPaths::ProjectContentDir() / TEXT("DefaultNPCs");
	IFileManager::Get().MakeDirectory(*FullDir, true);

	const FString PackageName = PackagePath + AssetName;
	UPackage* Package = CreatePackage(*PackageName);
	if (!Package)
	{
		UE_LOG(LogTemp, Error, TEXT("[NPCPlacementTool] Failed to create package: %s"), *PackageName);
		return NewObject<UNPCConfigDataAsset>(this);
	}
	Package->FullyLoad();

	UNPCConfigDataAsset* NewAsset = NewObject<UNPCConfigDataAsset>(Package, *AssetName, RF_Public | RF_Standalone);

	FNPCDefinition DefaultDef;
	DefaultDef.DisplayName = TEXT("NPC Type 1");
	DefaultDef.HotKey = 1;
	DefaultDef.Scale = FVector(1.0f);
	DefaultDef.DefaultRotation = FRotator::ZeroRotator;
	DefaultDef.SpawnDistance = 200.0f;
	NewAsset->NPCDefinitions.Add(DefaultDef);

	NewAsset->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(NewAsset);

	FString FilePath = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	bool bSaved = UPackage::SavePackage(Package, NewAsset, *FilePath, SaveArgs);

	if (bSaved)
	{
		UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] Created default config: %s"), *PackageName);
		FNotificationInfo Info(FText::FromString(
			TEXT("NPC Placement Tool: Created default config at /Game/DefaultNPCs/DA_DefaultNPCConfig\nOpen it to add your NPC types (meshes, animations).")));
		Info.ExpireDuration = 8.0f;
		Info.bUseLargeFont = false;
		FSlateNotificationManager::Get().AddNotification(Info);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[NPCPlacementTool] Failed to save config to disk, using transient copy"));
	}

	return NewAsset;
}

void UNPCPlacementToolSubsystem::StartPlacementMode()
{
	UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] Subsystem::StartPlacementMode - ENTRY, bIsInPlacementMode=%d"), bIsInPlacementMode);

	if (bIsInPlacementMode) { UE_LOG(LogTemp, Warning, TEXT("NPCPlacementTool: Already in placement mode")); return; }

	UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
	if (!EditorWorld) { UE_LOG(LogTemp, Error, TEXT("NPCPlacementTool: No editor world available")); return; }

	GEditor->Exec(EditorWorld, TEXT("SAVE"));

	PrePlacementSnapshot = NewObject<USceneSnapshot>(this);
	PrePlacementSnapshot->CaptureSnapshot(EditorWorld);

	if (!NPCConfig) NPCConfig = EnsureDefaultConfigExists();

	DiffCalculator = NewObject<USceneDiffCalculator>(this);
	bIsInPlacementMode = true;

	if (PostPIEStartedHandle.IsValid()) FEditorDelegates::PostPIEStarted.Remove(PostPIEStartedHandle);
	if (EndPIEHandle.IsValid()) FEditorDelegates::EndPIE.Remove(EndPIEHandle);

	PostPIEStartedHandle = FEditorDelegates::PostPIEStarted.AddLambda([this](bool bIsSimulating)
	{
		if (!bIsInPlacementMode) return;

		UWorld* PIEWorld = nullptr;
		if (GEngine)
		{
			const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
			for (const FWorldContext& WC : WorldContexts)
			{
				if (WC.WorldType == EWorldType::PIE && WC.World()) { PIEWorld = WC.World(); break; }
			}
		}
		if (!PIEWorld)
		{
			FWorldContext* PIEContext = GEditor->GetPIEWorldContext();
			if (PIEContext && PIEContext->World()) PIEWorld = PIEContext->World();
		}
		if (!PIEWorld) return;

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ANPCCoordinator* Coordinator = PIEWorld->SpawnActor<ANPCCoordinator>(ANPCCoordinator::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (Coordinator) Coordinator->Initialize(NPCConfig);
	});

	EndPIEHandle = FEditorDelegates::EndPIE.AddLambda([this](bool bIsSimulating)
	{
		if (!bIsInPlacementMode) return;
		bIsInPlacementMode = false;

		PendingNPCAdded.Empty();
		UWorld* PIEWorld = nullptr;
		if (GEngine)
		{
			const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
			for (const FWorldContext& WC : WorldContexts)
			{
				if (WC.WorldType == EWorldType::PIE && WC.World()) { PIEWorld = WC.World(); break; }
			}
		}
		if (PIEWorld)
		{
			TArray<AActor*> Coordinators;
			UGameplayStatics::GetAllActorsOfClass(PIEWorld, ANPCCoordinator::StaticClass(), Coordinators);
			for (AActor* Actor : Coordinators)
			{
				ANPCCoordinator* Coordinator = Cast<ANPCCoordinator>(Actor);
				if (Coordinator)
				{
					TArray<APlacedNPCActor*> NPCs = Coordinator->GetAllPlacedNPCs();
					for (APlacedNPCActor* NPC : NPCs)
					{
						if (NPC)
						{
							FActorDiff Diff = NPC->GetDiffFromOriginal();
							if (Diff.DiffType == EDiffType::Added) PendingNPCAdded.Add(Diff);
						}
					}
					break;
				}
			}
		}
		CalculateAndShowDiff();
		NPCConfig = nullptr;
	});

	FRequestPlaySessionParams Params;
	Params.WorldType = EPlaySessionWorldType::PlayInEditor;
	GEditor->RequestPlaySession(Params);
}

void UNPCPlacementToolSubsystem::StopPlacementMode()
{
	if (!bIsInPlacementMode) return;
	CalculateAndShowDiff();
	bIsInPlacementMode = false;
	NPCConfig = nullptr;
	GEditor->EndPlayMap();
}

void UNPCPlacementToolSubsystem::SaveCurrentScene()
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (World) GEditor->Exec(World, TEXT("SAVE"));
}

void UNPCPlacementToolSubsystem::CalculateAndShowDiff()
{
	if (!PrePlacementSnapshot || !DiffCalculator) return;

	UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
	if (!EditorWorld) return;

	TArray<FActorDiff> Diffs = DiffCalculator->CalculateDiff(PrePlacementSnapshot, EditorWorld);
	Diffs.Append(PendingNPCAdded);

	if (Diffs.Num() == 0) { UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] No differences found")); PendingNPCAdded.Empty(); return; }

	int32 Added = 0, Modified = 0, Deleted = 0;
	for (const FActorDiff& Diff : Diffs)
	{
		switch (Diff.DiffType)
		{
		case EDiffType::Added: Added++; break;
		case EDiffType::Modified: Modified++; break;
		case EDiffType::Deleted: Deleted++; break;
		default: break;
		}
	}

	FString DiffSummary = FString::Printf(TEXT("Found %d differences.\n\nAdded: %d\nModified: %d\nDeleted: %d\n\nApply to scene?"), Diffs.Num(), Added, Modified, Deleted);

	EAppReturnType::Type Result = FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(DiffSummary));

	if (Result == EAppReturnType::Yes)
	{
		DiffCalculator->ApplyDiffs(EditorWorld, Diffs);

		for (const FActorDiff& Diff : PendingNPCAdded)
		{
			if (Diff.DiffType != EDiffType::Added || !Diff.bIsNPC) continue;

			FNPCDefinition Def;
			if (!NPCConfig || !NPCConfig->GetNPCConfig(Diff.NPCConfigIndex, Def)) continue;

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			APlacedNPCActor* NewNPC = EditorWorld->SpawnActor<APlacedNPCActor>(APlacedNPCActor::StaticClass(), Diff.NewTransform.GetLocation(), Diff.NewTransform.GetRotation().Rotator(), SpawnParams);

			if (NewNPC)
			{
				NewNPC->SetActorScale3D(Diff.NewTransform.GetScale3D());
				NewNPC->InitializeFromConfig(Def, Diff.NPCConfigIndex);

				if (!Diff.NewAnimBlueprint.IsEmpty())
				{
					UClass* SavedABP = LoadObject<UClass>(nullptr, *Diff.NewAnimBlueprint);
					if (SavedABP && SavedABP->IsChildOf(UAnimInstance::StaticClass()))
					{
						NewNPC->SetAnimBlueprintClass(SavedABP);
						UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] Re-applied ABP: %s"), *Diff.NewAnimBlueprint);
					}
				}
				else if (!Diff.NewAnimation.IsEmpty())
				{
					UAnimSequence* SavedAnim = LoadObject<UAnimSequence>(nullptr, *Diff.NewAnimation);
					if (SavedAnim) { NewNPC->PlayAnimation(SavedAnim, true); UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] Re-applied anim: %s"), *Diff.NewAnimation); }
				}

				UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] Spawned NPC in editor: %s at %s"), *NewNPC->GetName(), *Diff.NewTransform.GetLocation().ToString());
			}
		}

		SaveCurrentScene();
		UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] Diffs applied and scene saved"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] Diffs discarded"));
	}

	PendingNPCAdded.Empty();
	PrePlacementSnapshot = nullptr;
}
