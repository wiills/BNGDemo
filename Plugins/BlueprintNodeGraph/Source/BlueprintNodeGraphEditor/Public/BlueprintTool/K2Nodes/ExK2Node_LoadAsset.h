// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/K2Nodes/ExK2Node_AsyncBase.h"
#include "ExK2Node_LoadAsset.generated.h"

/**
 * @class UExK2Node_LoadAsset
 * @brief 异步加载资产的蓝图节点
 * 
 * 在蓝图编辑器中提供异步加载资产的功能。
 * 支持按路径和按类加载两种模式。
 */
UCLASS()
class BLUEPRINTNODEGRAPHEDITOR_API UExK2Node_LoadAsset : public UExK2Node_AsyncBase
{
	GENERATED_BODY()

public:
	UExK2Node_LoadAsset(const FObjectInitializer& ObjectInitializer);

	virtual void AllocateDefaultPins() override;
	virtual FText GetMenuCategory() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* OutputGraph) override;
};
