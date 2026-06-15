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
				// SLG 公有头文件暴露 FScopedMessageScopeId 等类型，故为公有依赖。
				// Scope identity, local addressing and message isolation come from here.
				"ScopedMessageSystem",
			}
			);
	}
}
