#include "SceneDiffCalculator.h"
#include "SceneSnapshot.h"
#include "PlacedNPCActor.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

TArray<FActorDiff> USceneDiffCalculator::CalculateDiff(USceneSnapshot* BeforeSnapshot, UWorld* CurrentWorld)
{
	TArray<FActorDiff> Diffs;
	if (!BeforeSnapshot || !CurrentWorld) return Diffs;

	TArray<AActor*> CurrentActors;
	UGameplayStatics::GetAllActorsOfClass(CurrentWorld, AActor::StaticClass(), CurrentActors);

	// Check for new and modified actors
	for (AActor* Actor : CurrentActors)
	{
		if (!Actor) continue;

		APlacedNPCActor* NPC = Cast<APlacedNPCActor>(Actor);
		if (NPC)
		{
			FActorDiff Diff = NPC->GetDiffFromOriginal();
			if (Diff.DiffType != EDiffType::None)
			{
				Diffs.Add(Diff);
			}
		}
		else
		{
			// Non-NPC actor: check if modified
			FActorSnapshot OldSnapshot;
			if (BeforeSnapshot->FindSnapshot(Actor->GetName(), OldSnapshot))
			{
				if (HasTransformChanged(OldSnapshot.Transform, Actor->GetActorTransform()))
				{
					FActorDiff Diff;
					Diff.ActorName = Actor->GetName();
					Diff.ActorPath = Actor->GetPathName();
					Diff.DiffType = EDiffType::Modified;
					Diff.OldTransform = OldSnapshot.Transform;
					Diff.NewTransform = Actor->GetActorTransform();
					Diffs.Add(Diff);
				}
			}
		}
	}

	// Check for deleted actors
	for (const FActorSnapshot& Snapshot : BeforeSnapshot->GetSnapshots())
	{
		bool bFound = false;
		for (AActor* Actor : CurrentActors)
		{
			if (Actor && Actor->GetName() == Snapshot.ActorName)
			{
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			FActorDiff Diff;
			Diff.ActorName = Snapshot.ActorName;
			Diff.ActorPath = Snapshot.ActorPath;
			Diff.DiffType = EDiffType::Deleted;
			Diff.OldTransform = Snapshot.Transform;
			Diffs.Add(Diff);
		}
	}

	return Diffs;
}

bool USceneDiffCalculator::ApplyDiffs(UWorld* World, const TArray<FActorDiff>& Diffs)
{
	if (!World) return false;

	for (const FActorDiff& Diff : Diffs)
	{
		switch (Diff.DiffType)
		{
		case EDiffType::Modified:
		{
			// Find the actor and apply transform
			TArray<AActor*> AllActors;
			UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
			for (AActor* Actor : AllActors)
			{
				if (Actor && Actor->GetName() == Diff.ActorName)
				{
					Actor->SetActorTransform(Diff.NewTransform);
					break;
				}
			}
			break;
		}
		case EDiffType::Added:
		{
			// New NPC actors: keep them in the world (they were spawned in PIE)
			break;
		}
		case EDiffType::Deleted:
		{
			// Find and destroy
			TArray<AActor*> AllActors;
			UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
			for (AActor* Actor : AllActors)
			{
				if (Actor && Actor->GetName() == Diff.ActorName)
				{
					Actor->Destroy();
					break;
				}
			}
			break;
		}
		default:
			break;
		}
	}

	return true;
}

bool USceneDiffCalculator::HasTransformChanged(const FTransform& A, const FTransform& B, float Tolerance)
{
	return !A.Equals(B, Tolerance);
}
