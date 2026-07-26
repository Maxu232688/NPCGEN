#pragma once

#include "CoreMinimal.h"
#include "NPCConfigDataAsset.h"
#include "PlacedNPCActor.h"
#include "NPCSpawnManager.generated.h"

UCLASS()
class NPCPLACEMENTTOOL_API UNPCSpawnManager : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UWorld* InWorld, UNPCConfigDataAsset* InConfig);

	APlacedNPCActor* SpawnNPC(int32 NPCIndex, const FTransform& SpawnTransform);

	void RemoveNPC(APlacedNPCActor* NPC);

	TArray<APlacedNPCActor*> GetAllPlacedNPCs() const;

	void ClearAll();

private:
	UPROPERTY()
	UWorld* World = nullptr;

	UPROPERTY()
	UNPCConfigDataAsset* Config = nullptr;

	UPROPERTY()
	TArray<APlacedNPCActor*> PlacedNPCs;

	int32 SpawnCounter = 0;
};
