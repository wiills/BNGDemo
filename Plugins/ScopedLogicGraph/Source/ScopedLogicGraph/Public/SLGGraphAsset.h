// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "SLGTypes.h"
#include "SLGBakedGraph.h"
#include "SLGGraphAsset.generated.h"

class FObjectPreSaveContext;

/**
 * 图模板资产（编辑态）。
 * Authoring asset. Nodes are USTRUCTs stored as FInstancedStruct (not UObjects).
 * Bake() compiles the authoring data into a flat, cache-friendly, read-only baked form
 * that is shared across every scope instance.
 */
UCLASS(BlueprintType)
class SCOPEDLOGICGRAPH_API USLGGraphAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 玩法域：任务 / 战斗 / AI，驱动编辑器过滤与默认值。 */
	UPROPERTY(EditAnywhere, Category = "SLG")
	FGameplayTag Domain;

	/** 编辑态节点，每个元素包裹一个 FSLGNodeBase 派生结构。 */
	UPROPERTY(EditAnywhere, Category = "SLG", meta = (BaseStruct = "/Script/ScopedLogicGraph.SLGNodeBase", ExcludeBaseStruct))
	TArray<FInstancedStruct> Nodes;

	/** 连线（节点索引对）；Bake() 展开成拓扑 DAG。 */
	UPROPERTY(EditAnywhere, Category = "SLG")
	TArray<FSLGEdge> Edges;

	/** 子图：数据独立 Asset（决策 #3），编辑器提供 drill-in 统一入口。 */
	UPROPERTY(EditAnywhere, Category = "SLG")
	TArray<TObjectPtr<USLGGraphAsset>> SubGraphs;

	/** 编译产物：只读共享于所有 Scope 实例。 */
	UPROPERTY(Transient)
	FSLGBakedGraph Baked;

	/**
	 * 编译：编辑态 -> 烘焙态。最小实现做拓扑排序 + 实例偏移分配。
	 * Compile authoring data into the baked form. Returns true on success.
	 */
	bool Bake();

	/** 烘焙态是否可用。 */
	bool IsBaked() const { return Baked.IsValidBaked(); }

#if WITH_EDITOR
	/** 资产保存时自动烘焙（待定问题 #5 暂取"保存即烘焙"）。 */
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
#endif
};
