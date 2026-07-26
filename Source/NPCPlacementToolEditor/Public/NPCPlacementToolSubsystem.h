#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "DiffDataTypes.h"
#include "NPCPlacementToolSubsystem.generated.h"

class USceneSnapshot;
class USceneDiffCalculator;
class UNPCConfigDataAsset;

UCLASS()
class UNPCPlacementToolSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable)
	void StartPlacementMode();

	UFUNCTION(BlueprintCallable)
	void StopPlacementMode();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsInPlacementMode() const { return bIsInPlacementMode; }

private:
	UPROPERTY()
	USceneSnapshot* PrePlacementSnapshot = nullptr;

	UPROPERTY()
	USceneDiffCalculator* DiffCalculator = nullptr;

	UPROPERTY()
	UNPCConfigDataAsset* NPCConfig = nullptr;

	bool bIsInPlacementMode = false;

	TArray<FActorDiff> PendingNPCAdded;

	FDelegateHandle PostPIEStartedHandle;
	FDelegateHandle EndPIEHandle;

	void SaveCurrentScene();
	void CalculateAndShowDiff();

	UNPCConfigDataAsset* EnsureDefaultConfigExists();
};
