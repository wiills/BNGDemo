// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ExBlueprintNodeGraphDebugSubsystem.generated.h"

/** PIE/SIE 下调试气泡数据（编辑器模块侧轮询绘制）。 */
struct FExAsyncProxyDebugEntry
{
	TWeakObjectPtr<UObject> WeakOuter;
	TWeakObjectPtr<UObject> WeakProxy;
	FGuid NodeGuid;
	float StartWorldSeconds = 0.f;
	float TimeoutSeconds = 0.f;
	FString TipPrefix;
};

/**
 * ExK2 异步代理在等待期间的调试计时状态。
 * Delay 气泡依赖蓝图 latent linkage；SetK2NodeInfo 不在该链路内，故由本子系统 + 编辑器 SGraphNode 绘制同类气泡。
 */
UCLASS()
class BLUEPRINTNODEGRAPH_API UExBlueprintNodeGraphDebugSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void RegisterAsyncProxyBubble(UObject* OuterContext, UObject* ProxyObject, const FGuid& NodeGuid,
		float TimeoutSeconds, const FString& TipPrefix);
	void UnregisterAsyncProxyBubble(UObject* OuterContext, const FGuid& NodeGuid);

	bool HasActiveEntry(UObject* OuterContext, const FGuid& NodeGuid) const;

	/** OuterContext 通常为 Proxy->GetOuter()（蓝图 Self），须与调试焦点对象一致方可命中。 */
	bool TryGetBubbleText(UObject* OuterContext, const FGuid& NodeGuid, FString& OutText);

	/** 先按 PreferredOuter 命中；否则仅当存在唯一活跃条目与本 NodeGuid 匹配时显示（组件蓝图等）。 */
	bool TryGetBubbleTextAuto(UObject* PreferredOuter, const FGuid& NodeGuid, FString& OutText);

private:
	void CullStaleEntries();

	TMap<uint64, FExAsyncProxyDebugEntry> Entries;
};
