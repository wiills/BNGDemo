// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/Common/ExK2NodeTimeoutLatentAction.h"

#include "BlueprintTool/AsyncActions/ExBase_AsyncAction.h"
#include "BlueprintTool/Proxies/ExBase_FlowProxy.h"
#include "Engine/LatentActionManager.h"

#define LOCTEXT_NAMESPACE "ExK2NodeTimeoutLatentAction"

namespace ExK2NodeTimeoutLatentActionPrivate
{
	static int32 UUIDStringToLatentInt32(const FString& UUIDStr)
	{
		if (UUIDStr.IsEmpty())
		{
			return 0;
		}
		if (UUIDStr.IsNumeric())
		{
			return static_cast<int32>(FCString::Strtoui64(*UUIDStr, nullptr, 10));
		}
		return static_cast<int32>(GetTypeHash(UUIDStr));
	}
}

bool FExK2NodeTimeoutLatentAction::HasExistingForProxy(UObject* ProxyObject, const FString& UUIDStr)
{
	if (!ProxyObject)
	{
		return false;
	}

	UWorld* World = ProxyObject->GetWorld();
	UObject* CallbackTarget = ProxyObject->GetOuter();
	if (!World || !CallbackTarget)
	{
		return false;
	}

	const int32 LatentUUID = ExK2NodeTimeoutLatentActionPrivate::UUIDStringToLatentInt32(UUIDStr);
	if (LatentUUID == 0)
	{
		return false;
	}

	return World->GetLatentActionManager().FindExistingAction<FExK2NodeTimeoutLatentAction>(CallbackTarget,
		LatentUUID) != nullptr;
}

void FExK2NodeTimeoutLatentAction::TryRegister(UObject* ProxyObject, const FExLatentNodeInfo& NodeInfo)
{
	if (!ProxyObject || NodeInfo.TimeOut <= 0.f)
	{
		return;
	}

	UWorld* World = ProxyObject->GetWorld();
	if (!World)
	{
		return;
	}

	UObject* CallbackTarget = ProxyObject->GetOuter();
	if (!CallbackTarget)
	{
		return;
	}

	const int32 LatentUUID = ExK2NodeTimeoutLatentActionPrivate::UUIDStringToLatentInt32(NodeInfo.UUID);
	if (LatentUUID == 0)
	{
		return;
	}

	FLatentActionManager& Manager = World->GetLatentActionManager();
	if (Manager.FindExistingAction<FExK2NodeTimeoutLatentAction>(CallbackTarget, LatentUUID))
	{
		return;
	}

	FString Prefix;
	if (!NodeInfo.StartLog.IsEmpty())
	{
		Prefix = NodeInfo.StartLog;
	}
	else if (NodeInfo.TimeOut > 0.f)
	{
		Prefix = NSLOCTEXT("BlueprintNodeGraph", "ExK2NodeTimeoutDefaultTip", "Timeout").ToString();
	}
	else
	{
		Prefix = NSLOCTEXT("BlueprintNodeGraph", "ExK2NodeWaitingDefaultTip", "Waiting").ToString();
	}

	Manager.AddNewAction(CallbackTarget, LatentUUID,
		new FExK2NodeTimeoutLatentAction(ProxyObject, NodeInfo.TimeOut, MoveTemp(Prefix)));
}

FExK2NodeTimeoutLatentAction::FExK2NodeTimeoutLatentAction(UObject* ProxyObject, float TimeoutSeconds,
                                                           FString InTipPrefix)
	: WeakProxy(ProxyObject)
	  , TimeoutLimit(TimeoutSeconds)
	  , RemainingTime(TimeoutSeconds > 0.f ? TimeoutSeconds : 0.f)
	  , ElapsedTime(0.f)
	  , TipPrefix(MoveTemp(InTipPrefix))
{
}

void FExK2NodeTimeoutLatentAction::UpdateOperation(FLatentResponse& Response)
{
	UObject* Proxy = WeakProxy.Get();
	if (!Proxy)
	{
		Response.DoneIf(true);
		return;
	}

	bool bFinished = false;
	if (UExBase_AsyncAction* AsyncProxy = Cast<UExBase_AsyncAction>(Proxy))
	{
		bFinished = AsyncProxy->IsFinished();
	}
	else if (UExBase_FlowProxy* LatentProxy = Cast<UExBase_FlowProxy>(Proxy))
	{
		bFinished = LatentProxy->IsFinished();
	}

	if (bFinished)
	{
		Response.DoneIf(true);
		return;
	}

	const float DeltaSeconds = Response.ElapsedTime();

	if (TimeoutLimit > 0.f)
	{
		RemainingTime -= DeltaSeconds;
		if (RemainingTime <= 0.f)
		{
			InvokeTryFinish(Proxy);
			Response.DoneIf(true);
			return;
		}
	}
	else
	{
		ElapsedTime += DeltaSeconds;
	}

	Response.DoneIf(false);
}

void FExK2NodeTimeoutLatentAction::InvokeTryFinish(UObject* ProxyObject)
{
	if (UExBase_AsyncAction* AsyncProxy = Cast<UExBase_AsyncAction>(ProxyObject))
	{
		if (!AsyncProxy->IsFinished())
		{
			AsyncProxy->TryFinish();
		}
	}
	else if (UExBase_FlowProxy* LatentProxy = Cast<UExBase_FlowProxy>(ProxyObject))
	{
		if (!LatentProxy->IsFinished())
		{
			LatentProxy->TryFinish();
		}
	}
}

// 仅编辑器：蓝图 latent 调试 UI 读取此字符串绘制气泡；非编辑器配置无此函数，亦无节点气泡 overlay。
#if WITH_EDITOR
FString FExK2NodeTimeoutLatentAction::GetDescription() const
{
	static const FNumberFormattingOptions DelayTimeFormatOptions = FNumberFormattingOptions()
		                                                              .SetMinimumFractionalDigits(3)
		                                                              .SetMaximumFractionalDigits(3);

	if (TimeoutLimit > 0.f)
	{
		const FText SecondsLeft = FText::AsNumber(RemainingTime, &DelayTimeFormatOptions);
		return FText::Format(
			NSLOCTEXT("BlueprintNodeGraph", "ExK2NodeTimeoutDescFmt", "{0} ({1} seconds left)"),
			FText::FromString(TipPrefix),
			SecondsLeft).ToString();
	}

	const FText Elapsed = FText::AsNumber(ElapsedTime, &DelayTimeFormatOptions);
	return FText::Format(
		NSLOCTEXT("BlueprintNodeGraph", "ExK2NodeElapsedDescFmt", "{0} ({1} seconds elapsed)"),
		FText::FromString(TipPrefix),
		Elapsed).ToString();
}
#endif

#undef LOCTEXT_NAMESPACE
