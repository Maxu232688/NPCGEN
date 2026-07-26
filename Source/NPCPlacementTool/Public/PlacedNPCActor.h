#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NPCConfigDataAsset.h"
#include "DiffDataTypes.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimInstance.h"
#include "PlacedNPCActor.generated.h"

UCLASS()
class NPCPLACEMENTTOOL_API APlacedNPCActor : public AActor
{
	GENERATED_BODY()

public:
	APlacedNPCActor();

	void InitializeFromConfig(const FNPCDefinition& Config, int32 SpawnIndex);

	UFUNCTION(BlueprintCallable)
	void PlayAnimation(UAnimSequence* Anim, bool bLoop = true);

	UFUNCTION(BlueprintCallable)
	void SetAnimBlueprintClass(TSubclassOf<UAnimInstance> AnimClass);

	UFUNCTION(BlueprintCallable)
	void SetAnimationSpeed(float Speed);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UAnimSequence* GetCurrentAnimation() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	TSubclassOf<UAnimInstance> GetAnimBlueprintClass() const { return CurrentAnimBlueprintClass; }

	UFUNCTION(BlueprintCallable)
	void SetSelected(bool bSelected);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FActorDiff GetDiffFromOriginal() const;

	static TArray<UAnimSequence*> DiscoverAnimationsForMesh(
		USkeletalMesh* SkeletalMesh,
		const TArray<TSoftObjectPtr<UAnimSequence>>& PreconfiguredAnims);

	static TArray<UClass*> DiscoverAnimBlueprintsForMesh(
		USkeletalMesh* SkeletalMesh,
		const TArray<TSubclassOf<UAnimInstance>>& PreconfiguredABPs);

	USkeletalMeshComponent* GetMeshComponent() const { return MeshComponent; }

	int32 NPCIndex;

	FTransform OriginalTransform;
	FString OriginalAnimName;
	FString OriginalAnimBPClass;
	bool bIsNewActor = true;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(VisibleAnywhere)
	class UCapsuleComponent* SelectionCapsule;

	UPROPERTY()
	FNPCDefinition Config;

	UPROPERTY()
	UAnimSequence* CurrentAnimation;

	UPROPERTY()
	TSubclassOf<UAnimInstance> CurrentAnimBlueprintClass;

	bool bIsSelected = false;
};
