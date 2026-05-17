// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/ExLatentTaskBase.h"
#include "ExLatentTaskForAttach.generated.h"


/**
 * Latent Task, As Payload to other Actor|Object
 * CollapseCategories: hide all categories
 */
UCLASS(DefaultToInstanced,Abstract,HideDropdown,EditInlineNew,CollapseCategories)
class BLUEPRINTNODEGRAPH_API UExLatentTaskForAttach : public UExLatentTaskBase
{
	GENERATED_BODY()
};
