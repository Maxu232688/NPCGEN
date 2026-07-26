#include "PlacedNPCActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/AssetData.h"
#include "Animation/Skeleton.h"

APlacedNPCActor::APlacedNPCActor()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetRenderCustomDepth(true);
	MeshComponent->SetCustomDepthStencilValue(0);
	MeshComponent->SetSimulatePhysics(false);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMeshComponent->SetupAttachment(RootComponent);
	StaticMeshComponent->SetRenderCustomDepth(true);
	StaticMeshComponent->SetCustomDepthStencilValue(0);
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	StaticMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	StaticMeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	StaticMeshComponent->SetVisibility(false);

	SelectionCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("SelectionCapsule"));
	SelectionCapsule->SetupAttachment(RootComponent);
	SelectionCapsule->SetCapsuleHalfHeight(88.0f);
	SelectionCapsule->SetCapsuleRadius(34.0f);
	SelectionCapsule->SetRelativeLocation(FVector(0.0f, 0.0f, 88.0f));
	SelectionCapsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SelectionCapsule->SetCollisionObjectType(ECC_WorldDynamic);
	SelectionCapsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	SelectionCapsule->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	SelectionCapsule->SetHiddenInGame(true);
}

void APlacedNPCActor::BeginPlay()
{
	Super::BeginPlay();
	OriginalTransform = GetActorTransform();
}

void APlacedNPCActor::InitializeFromConfig(const FNPCDefinition& InConfig, int32 SpawnIndex)
{
	Config = InConfig;
	NPCIndex = SpawnIndex;

	SetActorScale3D(Config.Scale);
	SetActorRotation(Config.DefaultRotation);

	if (!Config.SkeletalMesh.IsNull())
	{
		USkeletalMesh* Mesh = Config.SkeletalMesh.LoadSynchronous();
		if (Mesh)
		{
			MeshComponent->SetSkeletalMesh(Mesh);
			MeshComponent->SetVisibility(true);
			StaticMeshComponent->SetVisibility(false);

			if (Config.DefaultAnimBlueprintClass)
				SetAnimBlueprintClass(Config.DefaultAnimBlueprintClass);
		}
	}
	else if (!Config.StaticMesh.IsNull())
	{
		UStaticMesh* SMesh = Config.StaticMesh.LoadSynchronous();
		if (SMesh)
		{
			StaticMeshComponent->SetStaticMesh(SMesh);
			StaticMeshComponent->SetVisibility(true);
			MeshComponent->SetVisibility(false);
		}
	}

	if (!Config.DefaultAnimation.IsNull() && !Config.DefaultAnimBlueprintClass)
	{
		UAnimSequence* Anim = Config.DefaultAnimation.LoadSynchronous();
		if (Anim)
		{
			PlayAnimation(Anim, true);
			OriginalAnimName = Anim->GetPathName();
		}
	}

	OriginalTransform = GetActorTransform();
	OriginalAnimBPClass = CurrentAnimBlueprintClass ? CurrentAnimBlueprintClass->GetPathName() : TEXT("");
	bIsNewActor = true;
}

void APlacedNPCActor::PlayAnimation(UAnimSequence* Anim, bool bLoop)
{
	if (!Anim || !MeshComponent) return;

	CurrentAnimation = Anim;
	MeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	MeshComponent->SetAnimation(Anim);
	MeshComponent->PlayAnimation(Anim, bLoop);

	UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] PlacedNPCActor::PlayAnimation - Playing '%s' (Loop=%d, Mode=%d)"), *Anim->GetName(), bLoop, (int32)MeshComponent->GetAnimationMode());
}

void APlacedNPCActor::SetAnimBlueprintClass(TSubclassOf<UAnimInstance> AnimClass)
{
	if (!MeshComponent || !AnimClass) return;

	CurrentAnimBlueprintClass = AnimClass;
	CurrentAnimation = nullptr;

	MeshComponent->SetAnimInstanceClass(AnimClass);
	MeshComponent->SetAnimationMode(EAnimationMode::AnimationBlueprint);

	UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] PlacedNPCActor::SetAnimBlueprintClass - Applied '%s'"), *AnimClass->GetName());
}

void APlacedNPCActor::SetAnimationSpeed(float Speed)
{
	if (MeshComponent)
		MeshComponent->GlobalAnimRateScale = Speed;
}

UAnimSequence* APlacedNPCActor::GetCurrentAnimation() const
{
	return CurrentAnimation;
}

void APlacedNPCActor::SetSelected(bool bSelected)
{
	bIsSelected = bSelected;
	if (MeshComponent)
	{
		MeshComponent->SetRenderCustomDepth(bSelected);
		MeshComponent->SetCustomDepthStencilValue(bSelected ? 1 : 0);
	}
	if (StaticMeshComponent)
	{
		StaticMeshComponent->SetRenderCustomDepth(bSelected);
		StaticMeshComponent->SetCustomDepthStencilValue(bSelected ? 1 : 0);
	}
}

FActorDiff APlacedNPCActor::GetDiffFromOriginal() const
{
	FActorDiff Diff;
	Diff.ActorName = GetName();
	Diff.ActorPath = GetPathName();
	Diff.bIsNPC = true;
	Diff.NPCConfigIndex = NPCIndex;

	FTransform CurrentTransform = GetActorTransform();
	FString CurrentAnimPath = CurrentAnimation ? CurrentAnimation->GetPathName() : TEXT("");
	FString CurrentABPPath = CurrentAnimBlueprintClass ? CurrentAnimBlueprintClass->GetPathName() : TEXT("");

	bool bTransformChanged = !CurrentTransform.Equals(OriginalTransform, 0.1f);
	bool bAnimChanged = CurrentAnimPath != OriginalAnimName;
	bool bABPChanged = CurrentABPPath != OriginalAnimBPClass;

	if (bIsNewActor)
	{
		Diff.DiffType = EDiffType::Added;
		Diff.NewTransform = CurrentTransform;
		Diff.NewAnimation = CurrentAnimPath;
		Diff.NewAnimBlueprint = CurrentABPPath;
	}
	else if (bTransformChanged || bAnimChanged || bABPChanged)
	{
		Diff.DiffType = EDiffType::Modified;
		Diff.OldTransform = OriginalTransform;
		Diff.NewTransform = CurrentTransform;
		Diff.OldAnimation = OriginalAnimName;
		Diff.NewAnimation = CurrentAnimPath;
		Diff.OldAnimBlueprint = OriginalAnimBPClass;
		Diff.NewAnimBlueprint = CurrentABPPath;
	}

	return Diff;
}

TArray<UAnimSequence*> APlacedNPCActor::DiscoverAnimationsForMesh(
	USkeletalMesh* SkeletalMesh,
	const TArray<TSoftObjectPtr<UAnimSequence>>& PreconfiguredAnims)
{
	UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] DiscoverAnimationsForMesh - Mesh=%s, Preconfigured count=%d"),
		SkeletalMesh ? *SkeletalMesh->GetName() : TEXT("null"), PreconfiguredAnims.Num());

	if (PreconfiguredAnims.Num() > 0)
	{
		TArray<UAnimSequence*> Result;
		for (const TSoftObjectPtr<UAnimSequence>& SoftAnim : PreconfiguredAnims)
		{
			if (!SoftAnim.IsNull())
			{
				UAnimSequence* Anim = SoftAnim.LoadSynchronous();
				if (Anim) Result.Add(Anim);
			}
		}
		UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] DiscoverAnimationsForMesh - Loaded %d preconfigured anims"), Result.Num());
		return Result;
	}

	if (!SkeletalMesh) return TArray<UAnimSequence*>();

	USkeleton* TargetSkeleton = SkeletalMesh->GetSkeleton();
	if (!TargetSkeleton) return TArray<UAnimSequence*>();

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	FARFilter Filter;
	Filter.ClassPaths.Add(UAnimSequence::StaticClass()->GetClassPathName());
	Filter.bRecursiveClasses = true;
	Filter.bRecursivePaths = true;

	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssets(Filter, AssetDataList);

	FString SkeletonPath = TargetSkeleton->GetPathName();
	TArray<UAnimSequence*> Result;

	for (const FAssetData& AssetData : AssetDataList)
	{
		FString AssetSkeletonPath;
		if (AssetData.GetTagValue(FName("Skeleton"), AssetSkeletonPath) && AssetSkeletonPath == SkeletonPath)
		{
			UAnimSequence* Anim = Cast<UAnimSequence>(AssetData.GetAsset());
			if (Anim) Result.Add(Anim);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] DiscoverAnimationsForMesh - Found %d skeleton-matched anims"), Result.Num());
	return Result;
}

TArray<UClass*> APlacedNPCActor::DiscoverAnimBlueprintsForMesh(
	USkeletalMesh* SkeletalMesh,
	const TArray<TSubclassOf<UAnimInstance>>& PreconfiguredABPs)
{
	UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] DiscoverAnimBlueprintsForMesh - Mesh=%s, Preconfigured ABPs=%d"),
		SkeletalMesh ? *SkeletalMesh->GetName() : TEXT("null"), PreconfiguredABPs.Num());

	if (PreconfiguredABPs.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] DiscoverAnimBlueprintsForMesh - Using %d preconfigured ABPs"), PreconfiguredABPs.Num());
		return TArray<UClass*>(PreconfiguredABPs);
	}

	if (!SkeletalMesh) return TArray<UClass*>();

	USkeleton* TargetSkeleton = SkeletalMesh->GetSkeleton();
	if (!TargetSkeleton) return TArray<UClass*>();

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	FARFilter Filter;
	Filter.ClassPaths.Add(FTopLevelAssetPath(TEXT("/Script/Engine"), TEXT("AnimBlueprint")));
	Filter.bRecursiveClasses = true;
	Filter.bRecursivePaths = true;

	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssets(Filter, AssetDataList);

	UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] DiscoverAnimBlueprintsForMesh - Found %d total AnimBlueprint assets"), AssetDataList.Num());

	FString SkeletonPath = TargetSkeleton->GetPathName();
	TArray<UClass*> Result;

	for (const FAssetData& AssetData : AssetDataList)
	{
		FString ABPSkeletonPath;
		if (AssetData.GetTagValue(FName("TargetSkeleton"), ABPSkeletonPath) && ABPSkeletonPath == SkeletonPath)
		{
			FString GeneratedClassName = AssetData.AssetName.ToString() + TEXT("_C");
			FString GeneratedClassPath = AssetData.PackageName.ToString() + TEXT(".") + GeneratedClassName;

			UClass* GeneratedClass = LoadObject<UClass>(nullptr, *GeneratedClassPath);
			if (GeneratedClass && GeneratedClass->IsChildOf(UAnimInstance::StaticClass()))
			{
				Result.Add(GeneratedClass);
				UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool]   Matched ABP: %s"), *GeneratedClass->GetName());
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[NPCPlacementTool] DiscoverAnimBlueprintsForMesh - Returning %d skeleton-matched ABPs"), Result.Num());
	return Result;
}
