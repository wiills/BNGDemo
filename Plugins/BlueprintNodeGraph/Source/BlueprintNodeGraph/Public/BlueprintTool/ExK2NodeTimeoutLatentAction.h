// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/ExLatentProxyDefine.h"
#include "LatentActions.h"

/**
 * ExK2 异步代理在 FLatentActionManager 中的调试 / 超时 latent（与 Delay 同源）。
 *
 * - TimeOut > 0：倒计时并在归零时 TryFinish（原有超时语义）。
 * - TimeOut <= 0：仅正向累计经过时间（Wait All / Wait Count 等未填超时仍有 PIE 气泡），不由本 latent 触发完成。
 *
 * 气泡文案仅在编辑器编译（WITH_EDITOR + GetDescription）；Shipping 无 overlay。
 */
class FExK2NodeTimeoutLatentAction final : public FPendingLatentAction
{
public:
	/** 与历史 SetK2NodeInfo 防重入一致：同一 Proxy + UUID 已存在本类 latent 时返回 true */
	static bool HasExistingForProxy(UObject* ProxyObject, const FString& UUIDStr);

	static void TryRegister(UObject* ProxyObject, const FExLatentNodeInfo& NodeInfo);

private:
	/** TimeoutSeconds：<= 0 为「仅累计耗时」模式；> 0 为倒计时并在结束时 TryFinish */
	FExK2NodeTimeoutLatentAction(UObject* ProxyObject, float TimeoutSeconds, FString InTipPrefix);

	virtual void UpdateOperation(FLatentResponse& Response) override;

#if WITH_EDITOR
	virtual FString GetDescription() const override;
#endif

	static void InvokeTryFinish(UObject* ProxyObject);

	TWeakObjectPtr<UObject> WeakProxy;
	float TimeoutLimit = 0.f;
	float RemainingTime = 0.f;
	float ElapsedTime = 0.f;
	FString TipPrefix;
};
