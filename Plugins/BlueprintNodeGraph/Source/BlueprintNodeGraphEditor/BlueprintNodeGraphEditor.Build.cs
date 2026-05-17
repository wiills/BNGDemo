// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BlueprintNodeGraphEditor : ModuleRules
{
	public BlueprintNodeGraphEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp20;
		
		// UE5.5+：`bEnableUndefinedIdentifierWarnings` 已弃用，改用 `UndefinedIdentifierWarningLevel`（原为 true → Warning）
		bUseUnity = true;
		UndefinedIdentifierWarningLevel = WarningLevel.Warning;
		bLegacyParentIncludePaths = false;
		
		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"BlueprintNodeGraph"
		});
		
		PrivateDependencyModuleNames.AddRange(new[]
		{
			"UnrealEd",
			"AssetTools",
			"Slate",
			"SlateCore",
			"GameplayTags",
			"UMG",
			"PlacementMode",
			"EditorStyle",
			"GraphEditor",
			"BlueprintGraph",
			"KismetCompiler"
		});
	}
}
