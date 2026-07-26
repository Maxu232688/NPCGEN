#pragma once

#include "CoreMinimal.h"
#include "SceneSnapshot.generated.h"

USTRUCT(BlueprintType)
struct NPCPLACEMENTTOOL_API FActorSnapshot
{
	GENERATED_BODY()

	UPROPERTY()
	FString ActorName;

	UPROPERTY()
	FString ActorPath;

	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	FString AnimationName;

	UPROPERTY()
	bool bIsNPC = false;

	UPROPERTY()
	int32 NPCConfigIndex = -1;
};

UCLASS()
class NPCPLACEMENTTOOL_API USceneSnapshot : public UObject
{
	GENERATED_BODY()

public:
	void CaptureSnapshot(UWorld* World);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<FActorSnapshot> GetSnapshots() const;

	bool FindSnapshot(const FString& ActorName, FActorSnapshot& OutSnapshot) const;

private:
	UPROPERTY()
	TArray<FActorSnapshot> Snapshots;

	FDateTime CaptureTime;
};
