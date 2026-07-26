#include "SceneSnapshot.h"
#include "PlacedNPCActor.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void USceneSnapshot::CaptureSnapshot(UWorld* World)
{
	Snapshots.Empty();
	if (!World) return;

	CaptureTime = FDateTime::Now();

	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

	for (AActor* Actor : AllActors)
	{
		if (!Actor) continue;

		FActorSnapshot Snap;
		Snap.ActorName = Actor->GetName();
		Snap.ActorPath = Actor->GetPathName();
		Snap.Transform = Actor->GetActorTransform();

		APlacedNPCActor* NPC = Cast<APlacedNPCActor>(Actor);
		if (NPC)
		{
			Snap.bIsNPC = true;
			Snap.AnimationName = NPC->GetCurrentAnimation() ? NPC->GetCurrentAnimation()->GetName() : TEXT("");
		}

		Snapshots.Add(Snap);
	}
}

TArray<FActorSnapshot> USceneSnapshot::GetSnapshots() const
{
	return Snapshots;
}

bool USceneSnapshot::FindSnapshot(const FString& ActorName, FActorSnapshot& OutSnapshot) const
{
	for (const FActorSnapshot& Snap : Snapshots)
	{
		if (Snap.ActorName == ActorName)
		{
			OutSnapshot = Snap;
			return true;
		}
	}
	return false;
}
