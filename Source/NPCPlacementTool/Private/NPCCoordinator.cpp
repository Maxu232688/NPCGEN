#include "NPCCoordinator.h"
#include "NPCPlacementCharacter.h"
#include "NPCSpawnManager.h"
#include "NPCConfigDataAsset.h"
#include "PlacedNPCActor.h"
#include "NPCPlacementWidget.h"
#include "Engine/World.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ANPCCoordinator::ANPCCoordinator()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ANPCCoordinator::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[NPCPlacementTool] Coordinator::BeginPlay - World is null!"));
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("[NPCPlacementTool] Coordinator::BeginPlay - No PlayerController!"));
		return;
	}

	APawn* ExistingPawn = PC->GetPawn();
	PlayerCharacter = Cast<ANPCPlacementCharacter>(ExistingPawn);

	if (!PlayerCharacter)
	{
		FVector CamLoc;
		FRotator CamRot;
		PC->GetPlayerViewPoint(CamLoc, CamRot);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		PlayerCharacter = World->SpawnActor<ANPCPlacementCharacter>(
			ANPCPlacementCharacter::StaticClass(), CamLoc, CamRot, SpawnParams);

		if (PlayerCharacter)
		{
			PC->Possess(PlayerCharacter);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[NPCPlacementTool] Coordinator::BeginPlay - FAILED to spawn character!"));
		}
	}

	SpawnManager = NewObject<UNPCSpawnManager>(this);
}

void ANPCCoordinator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	for (APlacedNPCActor* NPC : SelectedNPCs)
	{
		if (!NPC) continue;
		FVector Origin = NPC->GetActorLocation();
		DrawDebugCircle(GetWorld(), Origin, 60.0f, 32, FColor::Yellow, false, -1.0f, 0, 2.0f, FVector(0,1,0), FVector(1,0,0));
		DrawDebugLine(GetWorld(), Origin, Origin + FVector(0,0,200), FColor::Yellow, false, -1.0f, 0, 3.0f);
		DrawDebugSphere(GetWorld(), Origin + FVector(0,0,210), 15.0f, 8, FColor::Yellow, false, -1.0f, 0, 2.0f);
	}
}

void ANPCCoordinator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveUI();
	if (SpawnManager)
	{
		SpawnManager->ClearAll();
	}
	SelectedNPCs.Empty();
	Super::EndPlay(EndPlayReason);
}

void ANPCCoordinator::Initialize(UNPCConfigDataAsset* Config)
{
	NPCConfig = Config;
	if (SpawnManager && NPCConfig)
		SpawnManager->Initialize(GetWorld(), NPCConfig);
	CreateUI();
}

APlacedNPCActor* ANPCCoordinator::SpawnNPCAtPlayer(int32 NPCIndex)
{
	if (!PlayerCharacter || !SpawnManager) return nullptr;
	FTransform SpawnTransform = PlayerCharacter->GetActorTransform();
	FVector TraceStart = SpawnTransform.GetLocation();
	FVector TraceEnd = TraceStart - FVector(0.0f, 0.0f, 5000.0f);
	FHitResult Hit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(PlayerCharacter);
	QueryParams.AddIgnoredActor(this);
	if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
		SpawnTransform.SetLocation(Hit.Location);
	SpawnTransform.SetScale3D(FVector(1.0f));
	APlacedNPCActor* NPC = SpawnManager->SpawnNPC(NPCIndex, SpawnTransform);
	if (NPC)
	{
		UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] SpawnNPCAtPlayer - NPC spawned: %s"), *NPC->GetName());
		CurrentSelectedNPC = NPC;
		if (PlacementWidget.IsValid())
		{
			PlacementWidget->RefreshNPCList(SpawnManager->GetAllPlacedNPCs());
			PlacementWidget->SelectNPC(NPC);
		}
		if (PlayerCharacter)
			PlayerCharacter->SetMouseCapture(false);
	}
	return NPC;
}

void ANPCCoordinator::SelectNPCType(int32 Index)
{
	SelectedNPCIndex = Index;
	if (PlacementWidget.IsValid())
		PlacementWidget->SetSelectedNPCType(Index);
}

void ANPCCoordinator::UndoLastPlacement()
{
	if (!SpawnManager) return;
	TArray<APlacedNPCActor*> NPCs = SpawnManager->GetAllPlacedNPCs();
	if (NPCs.Num() > 0)
	{
		APlacedNPCActor* LastNPC = NPCs.Last();
		SelectedNPCs.Remove(LastNPC);
		if (CurrentSelectedNPC == LastNPC) CurrentSelectedNPC = nullptr;
		UpdateSelectionHighlight();
		SpawnManager->RemoveNPC(LastNPC);
		if (PlacementWidget.IsValid())
		{
			PlacementWidget->RefreshNPCList(SpawnManager->GetAllPlacedNPCs());
			PlacementWidget->ClearNPCPanel();
		}
	}
}

void ANPCCoordinator::SetSelectedNPC(APlacedNPCActor* NPC)
{
	CurrentSelectedNPC = NPC;
}

void ANPCCoordinator::RotateSelectedNPC(float DeltaDegrees)
{
	if (!CurrentSelectedNPC) return;
	FRotator CurrentRot = CurrentSelectedNPC->GetActorRotation();
	CurrentRot.Yaw += DeltaDegrees;
	CurrentSelectedNPC->SetActorRotation(CurrentRot);
}

void ANPCCoordinator::SelectPlacedNPC(APlacedNPCActor* NPC)
{
	if (!NPC) return;
	CurrentSelectedNPC = NPC;
	if (PlacementWidget.IsValid())
		PlacementWidget->SelectNPC(NPC);
}

void ANPCCoordinator::ToggleNPCSelection(APlacedNPCActor* NPC)
{
	if (!NPC) return;
	if (SelectedNPCs.Contains(NPC))
	{
		SelectedNPCs.Remove(NPC);
		UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] Deselected NPC: %s (selection count: %d)"), *NPC->GetName(), SelectedNPCs.Num());
	}
	else
	{
		SelectedNPCs.Add(NPC);
		UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] Selected NPC: %s (selection count: %d)"), *NPC->GetName(), SelectedNPCs.Num());
	}
	CurrentSelectedNPC = NPC;
	UpdateSelectionHighlight();
	if (PlacementWidget.IsValid())
		PlacementWidget->SelectNPC(NPC);
}

void ANPCCoordinator::SelectNPCsInScreenRect(const FVector2D& Start, const FVector2D& End, APlayerController* PC)
{
	if (!PC) return;
	FVector2D RectMin(FMath::Min(Start.X, End.X), FMath::Min(Start.Y, End.Y));
	FVector2D RectMax(FMath::Max(Start.X, End.X), FMath::Max(Start.Y, End.Y));
	TArray<APlacedNPCActor*> AllNPCs = GetAllPlacedNPCs();
	ClearSelection();
	for (APlacedNPCActor* NPC : AllNPCs)
	{
		if (!NPC) continue;
		FVector WorldPos = NPC->GetActorLocation();
		FVector2D ScreenPos;
		if (PC->ProjectWorldLocationToScreen(WorldPos, ScreenPos))
		{
			if (ScreenPos.X >= RectMin.X && ScreenPos.X <= RectMax.X && ScreenPos.Y >= RectMin.Y && ScreenPos.Y <= RectMax.Y)
				SelectedNPCs.Add(NPC);
		}
	}
	UpdateSelectionHighlight();
	UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] Box-select: %d NPCs"), SelectedNPCs.Num());
	if (SelectedNPCs.Num() > 0 && PlacementWidget.IsValid())
	{
		CurrentSelectedNPC = SelectedNPCs[0];
		PlacementWidget->SelectNPC(CurrentSelectedNPC);
	}
}

void ANPCCoordinator::ClearSelection()
{
	SelectedNPCs.Empty();
	UpdateSelectionHighlight();
}

void ANPCCoordinator::RotateSelectedNPCs(float DeltaDegrees)
{
	if (SelectedNPCs.Num() == 0) { RotateSelectedNPC(DeltaDegrees); return; }
	for (APlacedNPCActor* NPC : SelectedNPCs)
	{
		if (NPC)
		{
			FRotator CurrentRot = NPC->GetActorRotation();
			CurrentRot.Yaw += DeltaDegrees;
			NPC->SetActorRotation(CurrentRot);
		}
	}
	UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] Rotated %d selected NPCs by %.1f degrees"), SelectedNPCs.Num(), DeltaDegrees);
}

void ANPCCoordinator::DeleteSelectedNPCs()
{
	if (SelectedNPCs.Num() == 0 || !SpawnManager) return;
	int32 Count = SelectedNPCs.Num();
	for (APlacedNPCActor* NPC : SelectedNPCs)
	{
		if (NPC) SpawnManager->RemoveNPC(NPC);
	}
	if (CurrentSelectedNPC && SelectedNPCs.Contains(CurrentSelectedNPC))
		CurrentSelectedNPC = nullptr;
	SelectedNPCs.Empty();
	if (PlacementWidget.IsValid())
	{
		PlacementWidget->RefreshNPCList(SpawnManager->GetAllPlacedNPCs());
		PlacementWidget->ClearNPCPanel();
	}
	UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] Deleted %d selected NPCs"), Count);
}

void ANPCCoordinator::UpdateSelectionHighlight()
{
	TArray<APlacedNPCActor*> AllNPCs = GetAllPlacedNPCs();
	for (APlacedNPCActor* NPC : AllNPCs)
	{
		if (NPC) NPC->SetSelected(SelectedNPCs.Contains(NPC));
	}
}

TArray<APlacedNPCActor*> ANPCCoordinator::GetAllPlacedNPCs() const
{
	if (SpawnManager) return SpawnManager->GetAllPlacedNPCs();
	return TArray<APlacedNPCActor*>();
}

void ANPCCoordinator::CreateUI()
{
	if (!GEngine || !GEngine->GameViewport) { UE_LOG(LogTemp, Error, TEXT("[NPCPlacementTool] Coordinator::CreateUI - No GameViewport!")); return; }
	RemoveUI();
	PlacementWidget = SNew(SNPCPlacementWidget).NPCConfig(NPCConfig).Coordinator(this);
	GEngine->GameViewport->AddViewportWidgetContent(PlacementWidget.ToSharedRef());
	UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] Coordinator::CreateUI - Widget added"));
}

void ANPCCoordinator::RemoveUI()
{
	if (PlacementWidget.IsValid() && GEngine && GEngine->GameViewport)
		GEngine->GameViewport->RemoveViewportWidgetContent(PlacementWidget.ToSharedRef());
	PlacementWidget.Reset();
}
