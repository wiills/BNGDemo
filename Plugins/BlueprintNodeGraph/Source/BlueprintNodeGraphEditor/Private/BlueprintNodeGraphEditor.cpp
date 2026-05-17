// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintNodeGraphEditor/Public/BlueprintNodeGraphEditor.h"

//#include "IPlacementModeModule.h"

#define LOCTEXT_NAMESPACE "FBlueprintNodeGraphEditorModule"

void FBlueprintNodeGraphEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	// // Register actors to category
	// IPlacementModeModule::Get().RegisterPlaceableItem(LevelTools.UniqueHandle, MakeShareable(new FPlaceableItem(nullptr, FAssetData(AAISpawnPoint::StaticClass()))));
	// IPlacementModeModule::Get().RegisterPlaceableItem(LevelTools.UniqueHandle, MakeShareable(new FPlaceableItem(nullptr, FAssetData(ALevelTriggerBase::StaticClass()))));
}

void FBlueprintNodeGraphEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBlueprintNodeGraphEditorModule, BlueprintNodeGraphEditor)
