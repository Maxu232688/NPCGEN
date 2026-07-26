using UnrealBuildTool;

public class NPCPlacementTool : ModuleRules
{
	public NPCPlacementTool(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"UMG",
			"Slate",
			"SlateCore",
			"AssetRegistry"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});
	}
}
