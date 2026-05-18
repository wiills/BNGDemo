// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BlueprintNodeGraphEditor : ModuleRules
{
	public BlueprintNodeGraphEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		// 须与 .uplugin 中本模块 Type 一致：UncookedOnly，便于运行时蓝图引用 K2 节点且不将该模块打进打包运行时。
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
			"BlueprintNodeGraph",
			// 公开继承 UK2Node_* 的头文件需传递 BlueprintGraph，避免依赖方 LNK2019。
			"BlueprintGraph",
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
			"Kismet",
			// BlueprintGraph：K2 节点与蓝图编译管线。
			"BlueprintGraph",
			"KismetCompiler"
		});
	}
}
