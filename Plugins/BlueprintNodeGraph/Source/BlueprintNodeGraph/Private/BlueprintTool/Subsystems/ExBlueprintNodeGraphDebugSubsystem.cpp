// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/Subsystems/ExBlueprintNodeGraphDebugSubsystem.h"

#include "BlueprintTool/AsyncActions/ExBase_AsyncAction.h"
#include "BlueprintTool/Proxies/ExBase_FlowProxy.h"

#define LOCTEXT_NAMESPACE "ExBlueprintNodeGraphDebugSubsystem"

static uint64 MakeBubbleKey(UObject* OuterContext, const FGuid& NodeGuid)
{
	return HashCombine(GetTypeHash(NodeGuid), PointerHash(OuterContext));
}

static bool IsProxyFinished(UObject* ProxyObject)
{
	if (!ProxyObject)
	{
		return true;
	}
	if (const UExBase_AsyncAction* AsyncProxy = Cast<UExBase_AsyncAction>(ProxyObject))
	{
		return AsyncProxy->IsFinished();
	}
	if (const UExBase_FlowProxy* LatentProxy = Cast<UExBase_FlowProxy>(ProxyObject))
	{
		return LatentProxy->IsFinished();
	}
	return false;
}

static bool FormatBubbleText(UWorld* World, const FExAsyncProxyDebugEntry& Found, FString& OutText)
{
	if (!World)
	{
		return false;
	}

	static const FNumberFormattingOptions Opt = FNumberFormattingOptions()
		                                            .SetMinimumFractionalDigits(3)
		                                            .SetMaximumFractionalDigits(3);

	const float Now = World->GetTimeSeconds();
	const float Elapsed = FMath::Max(0.f, Now - Found.StartWorldSeconds);

	if (Found.TimeoutSeconds > 0.f)
	{
		const float Remaining = FMath::Max(0.f, Found.TimeoutSeconds - Elapsed);
		OutText = FText::Format(
			NSLOCTEXT("BlueprintNodeGraph", "ExLatentTimeoutDescFmt", "{0} ({1} seconds left)"),
			FText::FromString(Found.TipPrefix),
			FText::AsNumber(Remaining, &Opt)).ToString();
	}
	else
	{
		OutText = FText::Format(
			NSLOCTEXT("BlueprintNodeGraph", "ExLatentTimeoutElapsedDescFmt", "{0} ({1} seconds elapsed)"),
			FText::FromString(Found.TipPrefix),
			FText::AsNumber(Elapsed, &Opt)).ToString();
	}

	return true;
}

void UExBlueprintNodeGraphDebugSubsystem::RegisterAsyncProxyBubble(UObject* OuterContext, UObject* ProxyObject,
	const FGuid& NodeGuid, float TimeoutSeconds, const FString& TipPrefix)
{
	if (!OuterContext || !ProxyObject || !NodeGuid.IsValid())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FExAsyncProxyDebugEntry Entry;
	Entry.WeakOuter = OuterContext;
	Entry.WeakProxy = ProxyObject;
	Entry.NodeGuid = NodeGuid;
	Entry.StartWorldSeconds = World->GetTimeSeconds();
	Entry.TimeoutSeconds = TimeoutSeconds;
	Entry.TipPrefix = TipPrefix;

	Entries.Add(MakeBubbleKey(OuterContext, NodeGuid), Entry);
}

void UExBlueprintNodeGraphDebugSubsystem::UnregisterAsyncProxyBubble(UObject* OuterContext, const FGuid& NodeGuid)
{
	if (!OuterContext || !NodeGuid.IsValid())
	{
		return;
	}
	Entries.Remove(MakeBubbleKey(OuterContext, NodeGuid));
}

bool UExBlueprintNodeGraphDebugSubsystem::HasActiveEntry(UObject* OuterContext, const FGuid& NodeGuid) const
{
	if (!OuterContext || !NodeGuid.IsValid())
	{
		return false;
	}
	return Entries.Contains(MakeBubbleKey(OuterContext, NodeGuid));
}

void UExBlueprintNodeGraphDebugSubsystem::CullStaleEntries()
{
	for (auto It = Entries.CreateIterator(); It; ++It)
	{
		FExAsyncProxyDebugEntry& E = It.Value();
		UObject* Proxy = E.WeakProxy.Get();
		if (!Proxy || IsProxyFinished(Proxy))
		{
			It.RemoveCurrent();
		}
	}
}

bool UExBlueprintNodeGraphDebugSubsystem::TryGetBubbleText(UObject* OuterContext, const FGuid& NodeGuid,
	FString& OutText)
{
	CullStaleEntries();

	if (!OuterContext || !NodeGuid.IsValid())
	{
		return false;
	}

	FExAsyncProxyDebugEntry* Found = Entries.Find(MakeBubbleKey(OuterContext, NodeGuid));
	if (!Found)
	{
		return false;
	}

	UObject* Proxy = Found->WeakProxy.Get();
	if (!Proxy || IsProxyFinished(Proxy))
	{
		Entries.Remove(MakeBubbleKey(OuterContext, NodeGuid));
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	return FormatBubbleText(World, *Found, OutText);
}

bool UExBlueprintNodeGraphDebugSubsystem::TryGetBubbleTextAuto(UObject* PreferredOuter, const FGuid& NodeGuid,
	FString& OutText)
{
	if (PreferredOuter && TryGetBubbleText(PreferredOuter, NodeGuid, OutText))
	{
		return true;
	}

	CullStaleEntries();

	if (!NodeGuid.IsValid())
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const FExAsyncProxyDebugEntry* UniqueEntry = nullptr;
	int32 ActiveCount = 0;
	for (const auto& Pair : Entries)
	{
		const FExAsyncProxyDebugEntry& E = Pair.Value;
		if (E.NodeGuid != NodeGuid)
		{
			continue;
		}
		UObject* P = E.WeakProxy.Get();
		if (!P || IsProxyFinished(P))
		{
			continue;
		}
		UniqueEntry = &E;
		ActiveCount++;
		if (ActiveCount > 1)
		{
			return false;
		}
	}

	if (ActiveCount != 1 || !UniqueEntry)
	{
		return false;
	}

	return FormatBubbleText(World, *UniqueEntry, OutText);
}

#undef LOCTEXT_NAMESPACE
