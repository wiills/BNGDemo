// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BlueprintNodeGraph : ModuleRules
{
	public BlueprintNodeGraph(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp20;
		
		// UE5.5+：`bEnableUndefinedIdentifierWarnings` 已弃用，改用 `UndefinedIdentifierWarningLevel`（原为 true → Warning）
		bUseUnity = true;
		UndefinedIdentifierWarningLevel = WarningLevel.Warning;
		bLegacyParentIncludePaths = false;
		
		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core", "GameplayTags", "UMG"
		});
		
		PrivateDependencyModuleNames.AddRange(new[]
		{
			"CoreUObject",
			"Engine",
			"Json",
			"AssetRegistry",
			"Slate",
			"SlateCore",
			"GameplayTags",
			"GameplayTasks",
			"UMG"
		});
	}
}
