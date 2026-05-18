// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "K2Node_BaseAsyncTask.h"
#include "BlueprintTool/ExLatentProxyDefine.h"
#include "ExK2Node_AsyncBase.generated.h"

/**
 * @class UExK2Node_AsyncBase
 * @brief 异步节点编辑器基类
 * 
 * 继承自 UK2Node_BaseAsyncTask，为所有自定义异步节点提供基础功能，
 * 包括节点信息管理、UUID生成、编译器扩展等。
 * 
 * @see UK2Node_BaseAsyncTask, FExLatentNodeInfo
 */
UCLASS(Abstract)
class BLUEPRINTNODEGRAPHEDITOR_API UExK2Node_AsyncBase : public UK2Node_BaseAsyncTask
{
	GENERATED_BODY()

protected:
	/** SetK2NodeInfo 函数名称 */
	UPROPERTY()
	FName ProxySetK2NodeInfoFunctionName;

public:
	/** 节点配置信息 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NodeInfo")
	FExLatentNodeInfo NodeInfo;

	/**
	 * @brief 构造函数
	 * @param ObjectInitializer 对象初始化器
	 */
	UExK2Node_AsyncBase(const FObjectInitializer& ObjectInitializer);

	/** 工厂函数尚未设置时（如蓝图动作搜索/过滤阶段的临时节点）避免走引擎 BaseAsyncTask::GetTooltipText 中对空 UFunction 的 ensure。 */
	virtual FText GetTooltipText() const override;

protected:
	/**
	 * @brief 是否在细节面板显示节点属性
	 * @return 始终返回 true
	 */
	virtual bool ShouldShowNodeProperties() const override { return true; }

	/**
	 * @brief 属性编辑后回调
	 * @param PropertyChangedEvent 属性变更事件
	 */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	/**
	 * @brief 获取输入分支数量
	 * @return 执行引脚的连接数量
	 */
	FORCEINLINE int32 GetInputBranchCount() const
	{
		return GetExecPin()->LinkedTo.Num();
	}

	/**
	 * @brief 获取节点唯一标识
	 * @return 基于 NodeGuid 生成的 UUID 字符串
	 */
	FORCEINLINE FString GetNodeUUID() const
	{
		return FString::Format(TEXT("{0}"), { FStringFormatArg(GetTypeHash(NodeGuid)) });
	}

	/**
	 * @brief 设置节点信息到引脚默认值
	 * @param Schema 蓝图架构
	 * @param NodeInfoVarPin 目标引脚
	 */
	void SetNodeInfoPinValue(const UEdGraphSchema_K2* Schema, UEdGraphPin* NodeInfoVarPin);

	/**
	 * @brief 设置 UUID 和节点信息到对应引脚
	 * @param Schema 蓝图架构
	 */
	virtual void SetUUIDAndNodeInfo(const UEdGraphSchema_K2* Schema);

	/**
	 * @brief 生成上下文唯一ID
	 * 
	 * 结合节点UUID和Owner对象的唯一ID生成完整的唯一标识
	 */
	void GenerateContextUniqueId();

	/**
	 * @brief 扩展节点（编译器入口）
	 * @param CompilerContext 编译器上下文
	 * @param SourceGraph 源图
	 */
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	/**
	 * @brief 编译器：创建 SetK2NodeInfo 函数调用
	 * 
	 * 如果需要，创建调用 "SetK2NodeInfo" 的中间节点来设置代理对象的节点信息
	 * 
	 * @param CompilerContext 编译器上下文
	 * @param SourceGraph 源图
	 * @param OutProxyValidateNode 输出的验证节点
	 * @param ProxyObjectPin 代理对象引脚
	 * @param OutLastThenPin 输出的最后 Then 引脚
	 * @param bIsErrorFree 是否无错误
	 */
	void CompilerSetK2NodeInfoCall(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph, class UK2Node_IfThenElse*& OutProxyValidateNode,
		UEdGraphPin* const ProxyObjectPin, UEdGraphPin*& OutLastThenPin, bool& bIsErrorFree);

	/**
	 * @brief 编译器：创建 Activate 函数调用
	 * 
	 * 如果需要，创建调用 "Activate" 的中间节点来激活代理对象
	 * 
	 * @param CompilerContext 编译器上下文
	 * @param SourceGraph 源图
	 * @param OutProxyValidateNode 输出的验证节点
	 * @param ProxyObjectPin 代理对象引脚
	 * @param OutLastThenPin 输出的最后 Then 引脚
	 * @param bIsErrorFree 是否无错误
	 */
	void CompilerSetActiveCall(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph, class UK2Node_IfThenElse*& OutProxyValidateNode,
		UEdGraphPin* const ProxyObjectPin, UEdGraphPin*& OutLastThenPin, bool& bIsErrorFree);

	/**
	 * @brief 编译器：处理委托绑定
	 * 
	 * 为每个委托创建事件绑定，并实现赋值链
	 * 
	 * @param CompilerContext 编译器上下文
	 * @param SourceGraph 源图
	 * @param OutProxyValidateNode 输出的验证节点
	 * @param InCallCreateProxyObjectNode 创建代理对象的函数调用节点
	 * @param ProxyObjectPin 代理对象引脚
	 * @param VariableOutputs 输出参数与局部变量的配对列表
	 * @param OutLastThenPin 输出的最后 Then 引脚
	 * @param bIsErrorFree 是否无错误
	 */
	void CompilerHandleDelegates(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph, class UK2Node_IfThenElse*& OutProxyValidateNode,
		const class UK2Node_CallFunction* InCallCreateProxyObjectNode, UEdGraphPin* const ProxyObjectPin,
		const TArray<FBaseAsyncTaskHelper::FOutputPinAndLocalVariable>& VariableOutputs, UEdGraphPin*& OutLastThenPin, bool& bIsErrorFree);
};
