#pragma once

#include "CoreMinimal.h"
#include "DiffDataTypes.generated.h"

UENUM(BlueprintType)
enum class EDiffType : uint8
{
	None,
	Added,
	Modified,
	Deleted
};

USTRUCT(BlueprintType)
struct NPCPLACEMENTTOOL_API FActorDiff
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString ActorName;

	UPROPERTY(BlueprintReadWrite)
	FString ActorPath;

	UPROPERTY(BlueprintReadWrite)
	EDiffType DiffType = EDiffType::None;

	UPROPERTY(BlueprintReadWrite)
	FTransform OldTransform;

	UPROPERTY(BlueprintReadWrite)
	FTransform NewTransform;

	UPROPERTY(BlueprintReadWrite)
	FString OldAnimation;

	UPROPERTY(BlueprintReadWrite)
	FString NewAnimation;

	/** Object path of the AnimBlueprint class currently applied (e.g. /Game/.../ABP_Manny.ABP_Manny_C). */
	UPROPERTY(BlueprintReadWrite)
	FString NewAnimBlueprint;

	/** Original AnimBlueprint class path before modification. */
	UPROPERTY(BlueprintReadWrite)
	FString OldAnimBlueprint;

	UPROPERTY(BlueprintReadWrite)
	bool bIsNPC = false;

	UPROPERTY(BlueprintReadWrite)
	int32 NPCConfigIndex = -1;
};
