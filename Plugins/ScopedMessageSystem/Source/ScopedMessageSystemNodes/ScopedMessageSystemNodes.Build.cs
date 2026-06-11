using UnrealBuildTool;

public class ScopedMessageSystemNodes : ModuleRules
{
	public ScopedMessageSystemNodes(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"BlueprintGraph",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"KismetCompiler",
				"ScopedMessageSystem",
				"UnrealEd",
			}
		);
	}
}
