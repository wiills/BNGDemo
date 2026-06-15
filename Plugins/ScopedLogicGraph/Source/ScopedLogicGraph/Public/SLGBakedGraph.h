// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "SLGBakedGraph.generated.h"

/**
 * 烘焙态单节点描述：扁平配置 + 实例布局 + DAG 邻接。
 * One node in the baked graph. NodeConfig is the read-only, flyweight behavior +
 * static config shared by every scope instance; the per-instance mutable state (if any)
 * lives in FSLGInstanceData::NodeStateBlob at [InstanceOffset, InstanceOffset+InstanceSize).
 */
USTRUCT()
struct SCOPEDLOGICGRAPH_API FSLGBakedNode
{
	GENERATED_BODY()

	/** 无状态行为 + 静态配置（FSLGNodeBase 派生），所有实例共享只读。 */
	UPROPERTY()
	FInstancedStruct NodeConfig;

	/** 实例状态在 NodeStateBlob 中的字节偏移；无状态节点为 INDEX_NONE。 */
	UPROPERTY()
	int32 InstanceOffset = INDEX_NONE;

	/** 实例状态字节大小；无状态节点为 0。 */
	UPROPERTY()
	int32 InstanceSize = 0;

	/** 实例状态的反射类型，用于构造/析构 blob 中的非平凡字段；可空。 */
	UPROPERTY()
	TObjectPtr<UScriptStruct> InstanceType = nullptr;

	/** 后继节点索引（拓扑展开后的出边）。 */
	UPROPERTY()
	TArray<int32> Successors;

	bool HasInstanceState() const { return InstanceOffset != INDEX_NONE && InstanceSize > 0; }
};

/**
 * 编译产物：扁平节点数组 + 拓扑执行序 + 实例布局表，只读共享于所有 Scope 实例。
 * Compiled, read-only, cache-friendly form produced by USLGGraphAsset::Bake().
 */
USTRUCT()
struct SCOPEDLOGICGRAPH_API FSLGBakedGraph
{
	GENERATED_BODY()

	/** 扁平节点表（与编辑态 Nodes 同序，便于回溯）。 */
	UPROPERTY()
	TArray<FSLGBakedNode> Nodes;

	/** 拓扑排序后的执行顺序（M0 串行执行直接按此序跑）。 */
	UPROPERTY()
	TArray<int32> ExecutionOrder;

	/** 入口节点索引（无入边的根节点）。 */
	UPROPERTY()
	TArray<int32> EntryNodes;

	/** 实例状态 blob 总字节数，运行器据此一次性分配。 */
	UPROPERTY()
	int32 InstanceBlobSize = 0;

	/** 是否已成功烘焙。 */
	UPROPERTY()
	bool bIsBaked = false;

	void Reset()
	{
		Nodes.Reset();
		ExecutionOrder.Reset();
		EntryNodes.Reset();
		InstanceBlobSize = 0;
		bIsBaked = false;
	}

	bool IsValidBaked() const { return bIsBaked && Nodes.Num() == ExecutionOrder.Num(); }
};
