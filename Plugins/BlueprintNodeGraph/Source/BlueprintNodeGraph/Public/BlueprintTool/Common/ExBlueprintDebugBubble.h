// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

struct FExLatentNodeInfo;

/** SetK2NodeInfo / TryFinish 与调试子系统之间的薄封装 */
namespace ExBlueprintDebugBubble
{
	bool HasActiveRegistration(UObject* ProxyObject, const FExLatentNodeInfo& Info);

	void Register(UObject* ProxyObject, const FExLatentNodeInfo& Info);

	void Unregister(UObject* ProxyObject, const FExLatentNodeInfo& Info);
}
