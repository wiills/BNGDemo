// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SLGTypes.h"
#include "SLGExecContext.h"
#include "SLGNode.generated.h"

/**
 * 无状态节点行为基类（USTRUCT，非 UObject）。
 * Stateless node behavior base. Static config lives in derived struct fields and is
 * shared (flyweight) across all scope instances; per-instance mutable state lives in
 * FSLGInstanceData and is reached via FSLGExecContext, never stored on the node.
 */
USTRUCT()
struct SCOPEDLOGICGRAPH_API FSLGNodeBase
{
	GENERATED_BODY()

	virtual ~FSLGNodeBase() = default;

	/**
	 * 线程亲和性。默认 GameThread；纯计算无副作用节点可重写为 AnyThread。
	 * Off-GameThread is only allowed for side-effect-free pure compute.
	 */
	virtual ESLGThreadAffinity GetThreadAffinity() const { return ESLGThreadAffinity::GameThread; }

	/**
	 * 节点实例状态类型；返回非空时烘焙期为其在 blob 中分配槽。无状态节点返回 nullptr。
	 * Reflected type of this node's per-instance state, or nullptr if stateless.
	 */
	virtual const UScriptStruct* GetInstanceDataType() const { return nullptr; }

	/**
	 * 执行一次。const 表示行为无状态——一切可变状态经 Ctx 读写。
	 * Execute once. const enforces statelessness; mutate instance state via Ctx.
	 */
	virtual ESLGNodeStatus Execute(FSLGExecContext& Ctx) const { return ESLGNodeStatus::Succeeded; }

	/** 局部寻址：本节点需要操作 Scope 内哪个角色的 actor。 */
	UPROPERTY(EditAnywhere, Category = "Addressing")
	FGameplayTag TargetRole;
};

/**
 * 示例节点：等待固定时长。
 * Example node. Duration is shared static config; elapsed time is per-instance state.
 */
USTRUCT()
struct FSLGNode_WaitState
{
	GENERATED_BODY()

	/** 已累计时间（每实例一份）。 */
	UPROPERTY()
	float Elapsed = 0.f;
};

USTRUCT(meta = (DisplayName = "Wait"))
struct SCOPEDLOGICGRAPH_API FSLGNode_Wait : public FSLGNodeBase
{
	GENERATED_BODY()

	/** 等待时长（秒），静态共享配置。 */
	UPROPERTY(EditAnywhere, Category = "SLG")
	float Duration = 1.f;

	virtual const UScriptStruct* GetInstanceDataType() const override
	{
		return FSLGNode_WaitState::StaticStruct();
	}

	virtual ESLGNodeStatus Execute(FSLGExecContext& Ctx) const override;
};

/**
 * 策划就地逻辑慢路径：承载一个轻量 UObject 执行 BP 逻辑。
 * Designer escape hatch. Forced to GameThread and kept off the hot POD path; use sparingly.
 */
USTRUCT(meta = (DisplayName = "Blueprint Proxy"))
struct SCOPEDLOGICGRAPH_API FSLGNode_BlueprintProxy : public FSLGNodeBase
{
	GENERATED_BODY()

	/** BP 执行体类型。 */
	UPROPERTY(EditAnywhere, Category = "SLG")
	TSubclassOf<class USLGBlueprintNodeObject> NodeClass;

	virtual ESLGThreadAffinity GetThreadAffinity() const override { return ESLGThreadAffinity::GameThread; }

	virtual ESLGNodeStatus Execute(FSLGExecContext& Ctx) const override;
};
