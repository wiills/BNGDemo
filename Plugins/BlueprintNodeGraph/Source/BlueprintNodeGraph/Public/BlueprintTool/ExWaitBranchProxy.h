// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ExLatentActionManager.h"
#include "ExParallelProxy.h"
#include "ExWaitBranchProxy.generated.h"

/**
 * Wait Multi-Input Proxy
 * 与 UExParallelProxy 对齐的完成策略：All（全部输入已报告）/ Any（任一路成功）/ Count（成功路数达到阈值）。
 * 每条输入执行引脚默认调用 Activate() 视为成功；若需将某路记为失败请在该支调用 ReportBranchFailed()。
 */
UCLASS()
class BLUEPRINTNODEGRAPH_API UExWaitBranchProxy : public UExLatentActionProxyBase
{
	GENERATED_BODY()

public:
	/**
	 * CreateProxy Function - (UExK2Node_WaitBranch - pins 与参数顺序一致)
	 * InputCount：期望的输入分支数量（与节点上 Input Count 一致）
	 */
	UFUNCTION(BlueprintCallable, Category = "Utilities|FlowControl",
		meta = (WorldContext = "WorldContextObject", HidePin = "UUID,InputCount"))
	static UExWaitBranchProxy* CreateProxy(UObject* WorldContextObject, FString UUID, int32 InputCount,
		EExParallelMode CompletionMode, int32 RequiredSuccessCount = 1);

protected:
	virtual bool IsFinishAfterBranches() const override { return true; }

	virtual void HandleBranchReported(bool bSuccess) override;

	virtual void OnBranchesFinished() override;

private:
	void InitializeForRun(EExParallelMode InMode, int32 InRequiredSuccess, int32 ExpectedBranches);

	UPROPERTY()
	EExParallelMode CompletionMode = EExParallelMode::All;

	/** Count 模式下需要的最少成功分支数（与 Parallel 语义一致） */
	UPROPERTY()
	int32 RequiredSuccessCount = 1;

	int32 ReportsReceived = 0;
	int32 SuccessReceived = 0;
};
