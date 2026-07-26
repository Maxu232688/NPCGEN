#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlacedNPCActor.h"
#include "NPCCoordinator.generated.h"

class ANPCPlacementCharacter;
class UNPCSpawnManager;
class UNPCConfigDataAsset;
class APlacedNPCActor;
class SNPCPlacementWidget;
class APlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlacementStopped);

UCLASS(NotPlaceable, Transient)
class NPCPLACEMENTTOOL_API ANPCCoordinator : public AActor
{
	GENERATED_BODY()

public:
	ANPCCoordinator();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void Initialize(UNPCConfigDataAsset* Config);

	UFUNCTION(BlueprintCallable)
	APlacedNPCActor* SpawnNPCAtPlayer(int32 NPCIndex);

	UFUNCTION(BlueprintCallable)
	void SelectNPCType(int32 Index);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetSelectedNPCIndex() const { return SelectedNPCIndex; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UNPCSpawnManager* GetSpawnManager() const { return SpawnManager; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	ANPCPlacementCharacter* GetPlayerCharacter() const { return PlayerCharacter; }

	UFUNCTION(BlueprintCallable)
	void UndoLastPlacement();

	UFUNCTION(BlueprintCallable)
	void SetSelectedNPC(APlacedNPCActor* NPC);

	UFUNCTION(BlueprintCallable)
	void SelectPlacedNPC(APlacedNPCActor* NPC);

	UFUNCTION(BlueprintCallable)
	void ToggleNPCSelection(APlacedNPCActor* NPC);

	UFUNCTION(BlueprintCallable)
	void SelectNPCsInScreenRect(const FVector2D& Start, const FVector2D& End, APlayerController* PC);

	UFUNCTION(BlueprintCallable)
	void ClearSelection();

	UFUNCTION(BlueprintCallable)
	void RotateSelectedNPCs(float DeltaDegrees);

	UFUNCTION(BlueprintCallable)
	void RotateSelectedNPC(float DeltaDegrees);

	UFUNCTION(BlueprintCallable)
	void DeleteSelectedNPCs();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<APlacedNPCActor*> GetAllPlacedNPCs() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	const TArray<APlacedNPCActor*>& GetSelectedNPCs() const { return SelectedNPCs; }

	UPROPERTY(BlueprintAssignable)
	FOnPlacementStopped OnPlacementStopped;

private:
	UPROPERTY()
	ANPCPlacementCharacter* PlayerCharacter = nullptr;

	UPROPERTY()
	UNPCSpawnManager* SpawnManager = nullptr;

	UPROPERTY()
	UNPCConfigDataAsset* NPCConfig = nullptr;

	int32 SelectedNPCIndex = -1;

	UPROPERTY()
	TObjectPtr<APlacedNPCActor> CurrentSelectedNPC;

	UPROPERTY()
	TArray<TObjectPtr<APlacedNPCActor>> SelectedNPCs;

	void CreateUI();
	void RemoveUI();
	void UpdateSelectionHighlight();

	TSharedPtr<SNPCPlacementWidget> PlacementWidget;
};
