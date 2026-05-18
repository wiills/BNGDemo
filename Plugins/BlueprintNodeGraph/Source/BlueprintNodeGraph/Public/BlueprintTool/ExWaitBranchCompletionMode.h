// Copyright BlueprintNodeGraph. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ExWaitBranchCompletionMode.generated.h"

/** 仅用于 Wait Branch 多路汇入代理的完成策略。 */
UENUM(BlueprintType)
enum class EExWaitBranchCompletionMode : uint8
{
	All,
	Any,
	Count
};
