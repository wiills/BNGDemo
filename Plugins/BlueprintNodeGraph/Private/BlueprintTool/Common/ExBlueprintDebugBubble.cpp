// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/Common/ExBlueprintDebugBubble.h"

#include "BlueprintTool/Subsystems/ExBlueprintNodeGraphDebugSubsystem.h"
#include "BlueprintTool/Common/ExLatentProxyDefine.h"
#include "Engine/GameInstance.h"

bool ExBlueprintDebugBubble::HasActiveRegistration(UObject* ProxyObject, const FExLatentNodeInfo& Info)
{
	if (!ProxyObject)
	{
		return false;
	}

	FGuid NodeGuid;
	if (!FGuid::Parse(Info.GraphNodeGuid, NodeGuid))
	{
		return false;
	}

	UWorld* World = ProxyObject->GetWorld();
	if (!World || !World->GetGameInstance())
	{
		return false;
	}

	UObject* OuterContext = ProxyObject->GetOuter();
	if (!OuterContext)
	{
		return false;
	}

	const UExBlueprintNodeGraphDebugSubsystem* Sub = World->GetGameInstance()->GetSubsystem<
		UExBlueprintNodeGraphDebugSubsystem>();
	return Sub && Sub->HasActiveEntry(OuterContext, NodeGuid);
}

void ExBlueprintDebugBubble::Register(UObject* ProxyObject, const FExLatentNodeInfo& Info)
{
	if (!ProxyObject)
	{
		return;
	}

	FGuid NodeGuid;
	if (!FGuid::Parse(Info.GraphNodeGuid, NodeGuid))
	{
		return;
	}

	UWorld* World = ProxyObject->GetWorld();
	if (!World || !World->GetGameInstance())
	{
		return;
	}

	UObject* OuterContext = ProxyObject->GetOuter();
	if (!OuterContext)
	{
		return;
	}

	FString Prefix;
	if (!Info.StartLog.IsEmpty())
	{
		Prefix = Info.StartLog;
	}
	else if (Info.TimeOut > 0.f)
	{
		Prefix = NSLOCTEXT("BlueprintNodeGraph", "ExLatentTimeoutDefaultTip", "Timeout").ToString();
	}
	else
	{
		Prefix = NSLOCTEXT("BlueprintNodeGraph", "ExLatentTimeoutWaitingTip", "Waiting").ToString();
	}

	if (UExBlueprintNodeGraphDebugSubsystem* Sub = World->GetGameInstance()->GetSubsystem<
		UExBlueprintNodeGraphDebugSubsystem>())
	{
		Sub->RegisterAsyncProxyBubble(OuterContext, ProxyObject, NodeGuid, Info.TimeOut, Prefix);
	}
}

void ExBlueprintDebugBubble::Unregister(UObject* ProxyObject, const FExLatentNodeInfo& Info)
{
	if (!ProxyObject)
	{
		return;
	}

	FGuid NodeGuid;
	if (!FGuid::Parse(Info.GraphNodeGuid, NodeGuid))
	{
		return;
	}

	UWorld* World = ProxyObject->GetWorld();
	if (!World || !World->GetGameInstance())
	{
		return;
	}

	UObject* OuterContext = ProxyObject->GetOuter();
	if (!OuterContext)
	{
		return;
	}

	if (UExBlueprintNodeGraphDebugSubsystem* Sub = World->GetGameInstance()->GetSubsystem<
		UExBlueprintNodeGraphDebugSubsystem>())
	{
		Sub->UnregisterAsyncProxyBubble(OuterContext, NodeGuid);
	}
}
