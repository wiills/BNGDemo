// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SLGTypes.generated.h"

/**
 * 节点线程亲和性。
 * Thread affinity of a node. AnyThread nodes MUST be side-effect free:
 * no UObject create/destroy, no actor access, no messaging — pure POD compute only.
 */
UENUM(BlueprintType)
enum class ESLGThreadAffinity : uint8
{
	/** 只能在 GameThread 执行（触场景/发消息/访问 UObject）。 */
	GameThread,

	/** 可在 worker 线程并行执行（无副作用纯计算）。 */
	AnyThread
};

/**
 * 节点单次执行后的状态。
 * Result of a single node execution within one graph tick.
 */
UENUM(BlueprintType)
enum class ESLGNodeStatus : uint8
{
	/** 仍在推进，下一帧继续。 */
	Running,

	/** 成功完成，沿出边继续。 */
	Succeeded,

	/** 失败，按失败边/策略处理。 */
	Failed,

	/** 被外部中断。 */
	Aborted
};

/**
 * 图实例生命周期状态（随 POI 是否有人切换）。
 * Lifecycle state of a graph instance, driven by player interest in the POI.
 */
UENUM(BlueprintType)
enum class ESLGInstanceState : uint8
{
	/** 未实例化。 */
	Uninitialized,

	/** 已分配实例数据但暂停推进（POI 无人）。 */
	Dormant,

	/** 正在参与调度执行（POI 有人）。 */
	Active
};

/**
 * 编辑态连线：以节点索引对表示一条有向边。
 * Authoring edge as an index pair. Bake() flattens edges into a topological DAG.
 */
USTRUCT(BlueprintType)
struct FSLGEdge
{
	GENERATED_BODY()

	/** 源节点在 USLGGraphAsset::Nodes 中的索引。 */
	UPROPERTY(EditAnywhere, Category = "SLG")
	int32 FromNode = INDEX_NONE;

	/** 目标节点在 USLGGraphAsset::Nodes 中的索引。 */
	UPROPERTY(EditAnywhere, Category = "SLG")
	int32 ToNode = INDEX_NONE;

	/** 源节点出端口序号（多分支节点用，单出可留 0）。 */
	UPROPERTY(EditAnywhere, Category = "SLG")
	int32 FromPin = 0;

	bool IsValid() const { return FromNode != INDEX_NONE && ToNode != INDEX_NONE; }
};
