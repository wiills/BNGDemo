// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ScopedLogicGraph : ModuleRules
{
	public ScopedLogicGraph(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"GameplayTags",
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// Scope identity, local addressing and message isolation come from here.
				"ScopedMessageSystem",
			}
			);
	}
}
