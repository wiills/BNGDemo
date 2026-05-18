// Copyright BlueprintNodeGraph. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "K2Node_AddPinInterface.h"
#include "ExK2Node_Parallel.generated.h"

class FBlueprintActionDatabaseRegistrar;
class FKismetCompilerContext;
class FNodeHandlingFunctor;

/**
 * Parallel：与 Sequence 等价的 Then_0…Then_N 顺序执行；用 Count 整型控制 Then 数量（2–32）。
 *
 * 注意：不能继承 UK2Node_ExecutionSequence —— 该引擎类为 MinimalAPI，基类虚函数实现未对第三方模块导出，
 * 会在插件中链接失败（LNK2001）。此处按引擎 K2Node_ExecutionSequence 行为在插件内自行实现引脚与编译处理。
 */
UCLASS()
class BLUEPRINTNODEGRAPHEDITOR_API UExK2Node_Parallel : public UK2Node, public IK2Node_AddPinInterface
{
	GENERATED_BODY()

public:
	UExK2Node_Parallel(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin UEdGraphNode Interface
	virtual void AllocateDefaultPins() override;
	virtual void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins) override;
	virtual void NotifyPinConnectionListChanged(UEdGraphPin* Pin) override;
	virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;
	virtual FText GetTooltipText() const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	//~ End UEdGraphNode Interface

	//~ Begin UK2Node Interface
	virtual FNodeHandlingFunctor* CreateNodeHandler(FKismetCompilerContext& CompilerContext) const override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	virtual bool CanEverInsertExecutionPin() const override { return true; }
	virtual bool CanEverRemoveExecutionPin() const override { return true; }
	//~ End UK2Node Interface

	//~ Begin IK2Node_AddPinInterface
	virtual void AddInputPin() override;
	//~ End IK2Node_AddPinInterface

	/** 与引擎 UK2Node_ExecutionSequence 对齐的辅助 API（供 Sync / 删脚使用）。 */
	void RemovePinFromExecutionNode(UEdGraphPin* TargetPin);
	bool CanRemoveExecutionPin() const;
	UEdGraphPin* GetThenPinGivenIndex(int32 Index);

protected:
	void EnsureOutputCountPin();
	void SyncThenOutputsToCountPin();
	int32 GetOutputCountDefault() const;
	static int32 ResolveOutputCountFromCountPin(const UEdGraphPin* CountPin);
	int32 CountThenExecOutputs() const;
	void SetCountPinDefaultFromThenCount();

private:
	void AddThenOutputPin();

	FName GetUniquePinName();
	FName GetPinNameGivenIndex(int32 Index) const;
};
