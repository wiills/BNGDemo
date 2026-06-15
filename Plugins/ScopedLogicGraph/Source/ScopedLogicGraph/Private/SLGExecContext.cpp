// Copyright Epic Games, Inc. All Rights Reserved.

#include "SLGExecContext.h"
#include "SLGBakedGraph.h"
#include "SLGInstanceData.h"
#include "HAL/PlatformProcess.h"

void* FSLGExecContext::GetMutableNodeState() const
{
	if (!Baked || !Instance || !Baked->Nodes.IsValidIndex(CurrentNodeIndex))
	{
		return nullptr;
	}

	const FSLGBakedNode& Node = Baked->Nodes[CurrentNodeIndex];
	if (!Node.HasInstanceState() || !Instance->NodeStateBlob.IsValidIndex(Node.InstanceOffset))
	{
		return nullptr;
	}

	return Instance->NodeStateBlob.GetData() + Node.InstanceOffset;
}

bool FSLGExecContext::IsGameThread() const
{
	return ::IsInGameThread();
}
