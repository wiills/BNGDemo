// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "K2Node_BaseAsyncTask.h"
#include "BlueprintTool/Common/ExLatentProxyDefine.h"
#include "ExK2Node_AsyncBase.generated.h"

/**
 * Base editor class for custom async blueprint nodes (extends UK2Node_BaseAsyncTask).
 * Node info, UUID, compiler expansion, delegate wiring.
 */
UCLASS(Abstract)
class BLUEPRINTNODEGRAPHEDITOR_API UExK2Node_AsyncBase : public UK2Node_BaseAsyncTask
{
	GENERATED_BODY()

protected:
	/** SetK2NodeInfo function name on the proxy class */
	UPROPERTY()
	FName ProxySetK2NodeInfoFunctionName;

public:
	/** Per-node latent configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NodeInfo")
	FExLatentNodeInfo NodeInfo;

	UExK2Node_AsyncBase(const FObjectInitializer& ObjectInitializer);

	/** Safe tooltip when factory function is not configured yet */
	virtual FText GetTooltipText() const override;

	/** Debug timing bubble in PIE/SIE */
	virtual TSharedPtr<class SGraphNode> CreateVisualWidget() override;

	virtual bool ShouldShowNodeProperties() const override { return true; }
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	
protected:
	
	FORCEINLINE int32 GetInputBranchCount() const
	{
		return GetExecPin()->LinkedTo.Num();
	}

	FORCEINLINE FString GetNodeUUID() const
	{
		return FString::Format(TEXT("{0}"), { FStringFormatArg(GetTypeHash(NodeGuid)) });
	}

	void SetNodeInfoPinValue(const UEdGraphSchema_K2* Schema, UEdGraphPin* NodeInfoVarPin);
	virtual void SetUUIDAndNodeInfo(const UEdGraphSchema_K2* Schema);
	void GenerateContextUniqueId();

	void CompilerSetK2NodeInfoCall(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph, class UK2Node_IfThenElse*& OutProxyValidateNode,
		UEdGraphPin* const ProxyObjectPin, UEdGraphPin*& OutLastThenPin, bool& bIsErrorFree);

	void CompilerSetActiveCall(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph, class UK2Node_IfThenElse*& OutProxyValidateNode,
		UEdGraphPin* const ProxyObjectPin, UEdGraphPin*& OutLastThenPin, bool& bIsErrorFree);

	void CompilerHandleDelegates(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph, class UK2Node_IfThenElse*& OutProxyValidateNode,
		const class UK2Node_CallFunction* InCallCreateProxyObjectNode, UEdGraphPin* const ProxyObjectPin,
		const TArray<FBaseAsyncTaskHelper::FOutputPinAndLocalVariable>& VariableOutputs, UEdGraphPin*& OutLastThenPin, bool& bIsErrorFree);
};
