// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ScopedMessageTypes.h"

struct FSLGBakedGraph;
struct FSLGBakedNode;
struct FSLGInstanceData;
class USLGSubsystem;
class UWorld;

/**
 * 传给节点 Execute 的上下文（非 UObject，按值/引用传递，不参与 GC）。
 * Transient context handed to FSLGNodeBase::Execute. Carries the scope identity,
 * the mutable instance data, the read-only baked graph and infra handles. A node
 * mutates state ONLY through this context, keeping the behavior itself stateless.
 */
struct SCOPEDLOGICGRAPH_API FSLGExecContext
{
	/** 本图实例所属的 POI 作用域。 */
	FScopedMessageScopeId ScopeId;

	/** 只读烘焙态（行为/配置/DAG/布局）。 */
	const FSLGBakedGraph* Baked = nullptr;

	/** 本 Scope 的可变实例状态（POD blob + 黑板）。 */
	FSLGInstanceData* Instance = nullptr;

	/** 局部寻址 + 调度入口。 */
	TWeakObjectPtr<USLGSubsystem> Subsystem;

	/** 世界上下文（仅 GameThread 节点可安全使用）。 */
	TWeakObjectPtr<UWorld> World;

	/** 当前正在执行的节点索引（执行器逐节点写入）。 */
	int32 CurrentNodeIndex = INDEX_NONE;

	/**
	 * 取当前节点的实例状态裸指针；无状态节点返回 nullptr。
	 * Returns a typed pointer into NodeStateBlob for the current node, or nullptr.
	 */
	void* GetMutableNodeState() const;

	template <typename T>
	T* GetMutableNodeState() const
	{
		return static_cast<T*>(GetMutableNodeState());
	}

	bool IsGameThread() const;
};
