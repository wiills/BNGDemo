// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using System.IO;
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

		bool bWithMessageRouter = IsGameplayMessageRouterAvailable(Target);
		PublicDefinitions.Add($"WITH_QUEST_MESSAGE_ROUTER={(bWithMessageRouter ? 1 : 0)}");

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

		if (bWithMessageRouter)
		{
			PrivateDependencyModuleNames.Add("GameplayMessageRuntime");
		}
	}

	bool IsGameplayMessageRouterAvailable(ReadOnlyTargetRules Target)
	{
		const string PluginName = "GameplayMessageRouter";

		if (Target.DisablePlugins != null)
		{
			foreach (string Disabled in Target.DisablePlugins)
			{
				if (string.Equals(Disabled, PluginName, StringComparison.OrdinalIgnoreCase))
				{
					return false;
				}
			}
		}

		if (Target.EnablePlugins != null)
		{
			foreach (string Enabled in Target.EnablePlugins)
			{
				if (string.Equals(Enabled, PluginName, StringComparison.OrdinalIgnoreCase))
				{
					return true;
				}
			}
		}

		if (Target.ProjectFile != null)
		{
			bool? ProjectEnabled = ReadPluginEnabledFlag(Target.ProjectFile.FullName, PluginName);
			if (ProjectEnabled.HasValue)
			{
				return ProjectEnabled.Value;
			}

			string ProjectDir = Path.GetDirectoryName(Target.ProjectFile.FullName)!;
			string ProjectPlugin = Path.Combine(ProjectDir, "Plugins", PluginName, PluginName + ".uplugin");
			if (File.Exists(ProjectPlugin))
			{
				return true;
			}
		}

		string SiblingPlugin = Path.GetFullPath(
			Path.Combine(PluginDirectory, "..", "..", PluginName, PluginName + ".uplugin"));
		if (File.Exists(SiblingPlugin))
		{
			return true;
		}

		string EnginePlugin = Path.Combine(
			EngineDirectory, "Plugins", "Runtime", PluginName, PluginName + ".uplugin");
		return File.Exists(EnginePlugin);
	}

	static bool? ReadPluginEnabledFlag(string DescriptorPath, string PluginName)
	{
		if (!File.Exists(DescriptorPath))
		{
			return null;
		}

		string Text = File.ReadAllText(DescriptorPath);
		string Needle = "\"Name\": \"" + PluginName + "\"";
		int Index = Text.IndexOf(Needle, StringComparison.OrdinalIgnoreCase);
		if (Index < 0)
		{
			return null;
		}

		int BlockEnd = Text.IndexOf('}', Index);
		if (BlockEnd < 0)
		{
			return true;
		}

		string Block = Text.Substring(Index, BlockEnd - Index);
		if (Block.IndexOf("\"Enabled\": false", StringComparison.OrdinalIgnoreCase) >= 0)
		{
			return false;
		}

		if (Block.IndexOf("\"Enabled\": true", StringComparison.OrdinalIgnoreCase) >= 0)
		{
			return true;
		}

		return true;
	}
}
