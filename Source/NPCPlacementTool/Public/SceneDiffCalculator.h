#pragma once

#include "CoreMinimal.h"
#include "DiffDataTypes.h"
#include "SceneDiffCalculator.generated.h"

class USceneSnapshot;

UCLASS()
class NPCPLACEMENTTOOL_API USceneDiffCalculator : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	TArray<FActorDiff> CalculateDiff(USceneSnapshot* BeforeSnapshot, UWorld* CurrentWorld);

	UFUNCTION(BlueprintCallable)
	bool ApplyDiffs(UWorld* World, const TArray<FActorDiff>& Diffs);

private:
	bool HasTransformChanged(const FTransform& A, const FTransform& B, float Tolerance = 0.1f);
};
