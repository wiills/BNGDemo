// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "SLGInstanceData.generated.h"

/**
 * 每个 Scope 一份的运行态实例数据。
 * Per-scope runtime state. Node instance states are packed contiguously into a
 * single POD blob (addressed by baked offsets) so the whole instance is memcpy-able
 * for snapshot / rollback, matching the project's CLTypes rollback rule.
 */
USTRUCT()
struct SCOPEDLOGICGRAPH_API FSLGInstanceData
{
	GENERATED_BODY()

	/** 所有"有状态节点"的实例状态，按烘焙态偏移连续摆放。 */
	UPROPERTY()
	TArray<uint8> NodeStateBlob;

	/** 黑板：FInstancedStruct 键值，与 ScopedMessage 载荷同源（决策 #1）。 */
	UPROPERTY()
	TArray<FInstancedStruct> Blackboard;

	/** 是否已按烘焙态布局完成分配。 */
	bool bInitialized = false;

	void Reset()
	{
		NodeStateBlob.Reset();
		Blackboard.Reset();
		bInitialized = false;
	}
};
