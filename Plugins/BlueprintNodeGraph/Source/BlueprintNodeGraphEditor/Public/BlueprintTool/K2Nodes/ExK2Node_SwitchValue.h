// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/K2Nodes/ExK2Node_ShowBase.h"
#include "EdGraph/EdGraphNode.h"
#include "ExK2Node_SwitchValue.generated.h"

/**
 * Switch Mapping
 */
UCLASS()
class BLUEPRINTNODEGRAPHEDITOR_API UExK2Node_SwitchValue : public UK2Node_ShowBase
{
	GENERATED_BODY()

public:
	UExK2Node_SwitchValue(const FObjectInitializer& ObjectInitializer);
	
	/* UEdGraphNode interface */
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	// 重载Node样式
	virtual TSharedPtr<SGraphNode> CreateVisualWidget() override;
	/* UEdGraphNode interface */

	/* UK2Node interface */
	virtual FText GetMenuCategory() const override;
	// most important：把这个节点的创建操作添加到右键菜单中
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	/* UK2Node interface */

	// add pin
	virtual void AllocateDefaultPins() override;
	virtual void AddPinToNode() override;
	
protected:
	// Returns the exec output pin name for a given 0-based index
	virtual FName GetPinNameGivenIndex(int32 InIndex) const;
	
	// custom compile
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

private:
	int32 Index = 0;
};
