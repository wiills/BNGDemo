// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintNodeGraph.h"
#include "BlueprintTool/Common/ExLatentProxyDefine.h"

#define LOCTEXT_NAMESPACE "FBlueprintNodeGraphModule"

DEFINE_LOG_CATEGORY(LogBlueprintNodeGraph);
DEFINE_LOG_CATEGORY(LogLatentTask);
DEFINE_LOG_CATEGORY(LogAsyncAction);

void FBlueprintNodeGraphModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
}

void FBlueprintNodeGraphModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBlueprintNodeGraphModule, BlueprintNodeGraph)