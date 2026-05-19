// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BlueprintNodeGraph : ModuleRules
{
	public BlueprintNodeGraph(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp20;
		
		// UE5.6+：`UndefinedIdentifierWarningLevel` 已移至 `CppCompileWarningSettings`
		bUseUnity = true;
		CppCompileWarningSettings.UndefinedIdentifierWarningLevel = WarningLevel.Warning;
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
			"JsonUtilities",
			"AssetRegistry",
			"Slate",
			"SlateCore",
			"GameplayTags",
			"GameplayTasks",
			"UMG"
		});
	}
}
