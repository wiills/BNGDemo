// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BNGDemo : ModuleRules
{
	public BNGDemo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"BNGDemo",
			"BNGDemo/Variant_Platforming",
			"BNGDemo/Variant_Platforming/Animation",
			"BNGDemo/Variant_Combat",
			"BNGDemo/Variant_Combat/AI",
			"BNGDemo/Variant_Combat/Animation",
			"BNGDemo/Variant_Combat/Gameplay",
			"BNGDemo/Variant_Combat/Interfaces",
			"BNGDemo/Variant_Combat/UI",
			"BNGDemo/Variant_SideScrolling",
			"BNGDemo/Variant_SideScrolling/AI",
			"BNGDemo/Variant_SideScrolling/Gameplay",
			"BNGDemo/Variant_SideScrolling/Interfaces",
			"BNGDemo/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
