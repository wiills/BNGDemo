// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ExK2Node_AsyncBase.h"
#include "ExK2Node_StreamLevel.generated.h"

/**
 * @class UExK2Node_StreamLevel
 * @brief 异步加载/卸载关卡的蓝图节点
 * 
 * 在蓝图编辑器中提供异步加载和卸载关卡的功能。
 * 支持延迟执行和完成回调。
 */
UCLASS()
class BLUEPRINTNODEGRAPHEDITOR_API UExK2Node_StreamLevel : public UExK2Node_AsyncBase
{
	GENERATED_BODY()

public:
	UExK2Node_StreamLevel(const FObjectInitializer& ObjectInitializer);

	virtual void AllocateDefaultPins() override;
	virtual FText GetMenuCategory() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* OutputGraph) override;
};
