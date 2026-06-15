// Copyright Epic Games, Inc. All Rights Reserved.

#include "SLGGraphAsset.h"
#include "SLGNode.h"
#include "SLGSubsystem.h"

#if WITH_EDITOR
#include "UObject/ObjectSaveContext.h"
#endif

namespace
{
	/** 返回某实例状态结构的对齐字节数（无自定义 ops 时退回默认对齐）。 */
	int32 GetStructAlignment(const UScriptStruct* Struct)
	{
		if (Struct)
		{
			if (UScriptStruct::ICppStructOps* Ops = Struct->GetCppStructOps())
			{
				return FMath::Max(1, Ops->GetAlignment());
			}
		}
		return DEFAULT_ALIGNMENT;
	}
}

bool USLGGraphAsset::Bake()
{
	Baked.Reset();

	const int32 NodeCount = Nodes.Num();
	if (NodeCount == 0)
	{
		UE_LOG(LogSLG, Warning, TEXT("Bake skipped: graph '%s' has no nodes."), *GetName());
		return false;
	}

	// 1) 拷贝节点配置到扁平烘焙数组，记录每个有状态节点的实例布局。
	Baked.Nodes.Reserve(NodeCount);
	int32 RunningOffset = 0;
	for (int32 Index = 0; Index < NodeCount; ++Index)
	{
		FSLGBakedNode BakedNode;
		BakedNode.NodeConfig = Nodes[Index];

		if (const FSLGNodeBase* Node = Nodes[Index].GetPtr<FSLGNodeBase>())
		{
			if (const UScriptStruct* StateType = Node->GetInstanceDataType())
			{
				const int32 Alignment = GetStructAlignment(StateType);
				RunningOffset = Align(RunningOffset, Alignment);

				BakedNode.InstanceOffset = RunningOffset;
				BakedNode.InstanceSize = StateType->GetStructureSize();
				BakedNode.InstanceType = const_cast<UScriptStruct*>(StateType);

				RunningOffset += BakedNode.InstanceSize;
			}
		}
		else
		{
			UE_LOG(LogSLG, Warning, TEXT("Bake: node %d in '%s' is not an FSLGNodeBase."), Index, *GetName());
		}

		Baked.Nodes.Add(MoveTemp(BakedNode));
	}
	Baked.InstanceBlobSize = RunningOffset;

	// 2) 由连线构建邻接表与入度，做 Kahn 拓扑排序。
	TArray<int32> InDegree;
	InDegree.Init(0, NodeCount);
	for (const FSLGEdge& Edge : Edges)
	{
		if (!Edge.IsValid() || !Baked.Nodes.IsValidIndex(Edge.FromNode) || !Baked.Nodes.IsValidIndex(Edge.ToNode))
		{
			UE_LOG(LogSLG, Warning, TEXT("Bake: dropping invalid edge %d->%d in '%s'."), Edge.FromNode, Edge.ToNode, *GetName());
			continue;
		}
		Baked.Nodes[Edge.FromNode].Successors.Add(Edge.ToNode);
		++InDegree[Edge.ToNode];
	}

	TArray<int32> Frontier;
	for (int32 Index = 0; Index < NodeCount; ++Index)
	{
		if (InDegree[Index] == 0)
		{
			Frontier.Add(Index);
			Baked.EntryNodes.Add(Index);
		}
	}

	Baked.ExecutionOrder.Reserve(NodeCount);
	while (Frontier.Num() > 0)
	{
		const int32 Current = Frontier.Pop(EAllowShrinking::No);
		Baked.ExecutionOrder.Add(Current);

		for (int32 Next : Baked.Nodes[Current].Successors)
		{
			if (--InDegree[Next] == 0)
			{
				Frontier.Add(Next);
			}
		}
	}

	// 3) 执行序节点数不足说明存在环，烘焙失败。
	if (Baked.ExecutionOrder.Num() != NodeCount)
	{
		UE_LOG(LogSLG, Error, TEXT("Bake failed: graph '%s' contains a cycle (%d/%d nodes ordered)."),
			*GetName(), Baked.ExecutionOrder.Num(), NodeCount);
		Baked.Reset();
		return false;
	}

	Baked.bIsBaked = true;
	UE_LOG(LogSLG, Log, TEXT("Baked graph '%s': %d nodes, %d instance bytes."),
		*GetName(), NodeCount, Baked.InstanceBlobSize);
	return true;
}

#if WITH_EDITOR
void USLGGraphAsset::PreSave(FObjectPreSaveContext SaveContext)
{
	if (!SaveContext.IsProceduralSave())
	{
		Bake();
	}
	Super::PreSave(SaveContext);
}
#endif
