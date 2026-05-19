// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/Common/ExLatentProxyDefine.h"
#include "LatentActions.h"

/**
 * 异步代理在 FLatentActionManager 中的超时 / 调试 latent（与 FExWaitAction 同类，非 K2 节点）。
 *
 * - TimeOut > 0：倒计时并在归零时 TryFinish。
 * - TimeOut <= 0：仅累计经过时间（Wait All / Wait Count 等未填超时仍有 PIE 气泡）。
 *
 * 气泡文案仅在编辑器（WITH_EDITOR + GetDescription）；Shipping 无 overlay。
 */
class FExLatentTimeoutAction final : public FPendingLatentAction
{
public:
	/** 同一 Proxy + UUID 已存在本类 latent 时返回 true */
	static bool HasExistingForProxy(UObject* ProxyObject, const FString& UUIDStr);

	static void TryRegister(UObject* ProxyObject, const FExLatentNodeInfo& NodeInfo);

private:
	FExLatentTimeoutAction(UObject* ProxyObject, float TimeoutSeconds, FString InTipPrefix);

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
