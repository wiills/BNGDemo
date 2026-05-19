// Copyright BlueprintNodeGraph. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ExBranchMode.generated.h"

/** Wait Branch 多路汇入完成策略：All / Any / Count。 */
UENUM(BlueprintType)
enum class EExBranchMode : uint8
{
	All,
	Any,
	Count
};
