// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ExK2Node_ShowBase.generated.h"

/**
 * 必要的基础实现 用于蓝图显示: GetMenuActions必要
 */
UCLASS(Abstract)
class BLUEPRINTNODEGRAPHEDITOR_API UK2Node_ShowBase : public UK2Node
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString NodeDesc;
	
	/* UEdGraphNode interface */
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual bool ShouldShowNodeProperties() const override { return true; }
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
	virtual void AddPinToNode() {}
	
	/** Custom In Out Pin DataType **/
	UEdGraphPin* GetThenPin() const; // default output pin
};
