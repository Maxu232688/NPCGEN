#include "NPCSpawnManager.h"
#include "PlacedNPCActor.h"
#include "Engine/World.h"

void UNPCSpawnManager::Initialize(UWorld* InWorld, UNPCConfigDataAsset* InConfig)
{
	World = InWorld;
	Config = InConfig;
	SpawnCounter = 0;
}

APlacedNPCActor* UNPCSpawnManager::SpawnNPC(int32 NPCIndex, const FTransform& SpawnTransform)
{
	if (!World || !Config)
	{
		UE_LOG(LogTemp, Warning, TEXT("NPCPlacementTool: SpawnNPC failed - World=%s, Config=%s"),
			World ? TEXT("valid") : TEXT("null"),
			Config ? TEXT("valid") : TEXT("null"));
		return nullptr;
	}

	FNPCDefinition NPCConfig;
	if (!Config->GetNPCConfig(NPCIndex, NPCConfig))
	{
		UE_LOG(LogTemp, Warning, TEXT("NPCPlacementTool: SpawnNPC failed - NPCIndex %d not found in config (total definitions: %d)"),
			NPCIndex, Config->NPCDefinitions.Num());
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = *FString::Printf(TEXT("NPC_%s_%d"), *NPCConfig.DisplayName, SpawnCounter++);

	APlacedNPCActor* NPC = World->SpawnActor<APlacedNPCActor>(
		APlacedNPCActor::StaticClass(),
		SpawnTransform,
		SpawnParams
	);

	if (NPC)
	{
		NPC->InitializeFromConfig(NPCConfig, NPCIndex);
		PlacedNPCs.Add(NPC);
	}

	return NPC;
}

void UNPCSpawnManager::RemoveNPC(APlacedNPCActor* NPC)
{
	if (NPC)
	{
		PlacedNPCs.Remove(NPC);
		NPC->Destroy();
	}
}

TArray<APlacedNPCActor*> UNPCSpawnManager::GetAllPlacedNPCs() const
{
	TArray<APlacedNPCActor*> Result;
	for (APlacedNPCActor* NPC : PlacedNPCs)
	{
		if (NPC)
		{
			Result.Add(NPC);
		}
	}
	return Result;
}

void UNPCSpawnManager::ClearAll()
{
	for (APlacedNPCActor* NPC : PlacedNPCs)
	{
		if (NPC)
		{
			NPC->Destroy();
		}
	}
	PlacedNPCs.Empty();
	SpawnCounter = 0;
}
