#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimInstance.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "NPCConfigDataAsset.generated.h"

USTRUCT(BlueprintType)
struct NPCPLACEMENTTOOL_API FNPCDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	TSoftObjectPtr<UStaticMesh> StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftObjectPtr<UAnimSequence> DefaultAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TArray<TSoftObjectPtr<UAnimSequence>> AvailableAnimations;

	/** Default AnimBlueprint (state machine) class to apply. Leave empty to auto-discover. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSubclassOf<UAnimInstance> DefaultAnimBlueprintClass;

	/** Pre-configured list of AnimBlueprint classes for this NPC type. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TArray<TSubclassOf<UAnimInstance>> AvailableAnimBlueprints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (ClampMin = "1", ClampMax = "9"))
	int32 HotKey = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
	FVector Scale = FVector(1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
	FRotator DefaultRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	float SpawnDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSoftObjectPtr<UTexture2D> Icon;
};

UCLASS(BlueprintType)
class NPCPLACEMENTTOOL_API UNPCConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Config")
	TArray<FNPCDefinition> NPCDefinitions;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetNPCConfig(int32 Index, FNPCDefinition& OutConfig) const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetNPCByHotKey(int32 HotKey, FNPCDefinition& OutConfig) const;
};
