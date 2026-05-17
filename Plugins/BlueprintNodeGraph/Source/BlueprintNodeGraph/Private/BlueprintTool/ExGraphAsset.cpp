// Copyright BlueprintNodeGraph. All Rights Reserved.

#include "BlueprintTool/ExGraphAsset.h"

void UExGraphAsset::AddEntryPoint(FName EntryName)
{
	if (!EntryPointNames.Contains(EntryName))
	{
		EntryPointNames.Add(EntryName);
	}
}

void UExGraphAsset::RemoveEntryPoint(FName EntryName)
{
	EntryPointNames.Remove(EntryName);
}
