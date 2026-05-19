// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/LatentTasks/ExBase_LatentTask.h"
#include "ExLatentTask_ForAttach.generated.h"


/**
 * Latent Task, As Payload to other Actor|Object
 * CollapseCategories: hide all categories
 * 仅作内联载荷，不可作为蓝图父类。
 */
UCLASS(DefaultToInstanced, Abstract, HideDropdown, NotBlueprintable, NotBlueprintType, EditInlineNew, CollapseCategories)
class BLUEPRINTNODEGRAPH_API UExLatentTask_ForAttach : public UExBase_LatentTask
{
	GENERATED_BODY()
};
