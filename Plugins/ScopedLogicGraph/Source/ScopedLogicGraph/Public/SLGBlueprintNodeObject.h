// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ScopedMessageTypes.h"
#include "SLGTypes.h"
#include "SLGBlueprintNodeObject.generated.h"

/**
 * 慢路径节点的蓝图执行体。
 * Lightweight UObject host for designer-authored node logic, driven by
 * FSLGNode_BlueprintProxy. Always runs on GameThread. Intentionally thin so the
 * data-oriented hot path stays free of per-node UObjects.
 */
UCLASS(Blueprintable, Abstract)
class SCOPEDLOGICGRAPH_API USLGBlueprintNodeObject : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * 蓝图实现的节点逻辑。
	 * Blueprint-implemented node body. Return the resulting node status.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "SLG", meta = (DisplayName = "Execute"))
	ESLGNodeStatus BP_Execute(const FScopedMessageScopeId& ScopeId);
};
