// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ExLatentActionManager.h"
#include "ExWaitBranchCompletionMode.h"
#include "ExWaitBranchProxy.generated.h"

/**
 * Wait Multi-Input Proxy
 * All（全部输入已报告）/ Any（任一路成功）/ Count（成功路数达到阈值）；单次输入对应一次 Activate / ReportBranchFailed。
 * 每条输入执行引脚默认调用 Activate() 视为成功；若需将某路记为失败请在该支调用 ReportBranchFailed()。
 */
UCLASS()
class BLUEPRINTNODEGRAPH_API UExWaitBranchProxy : public UExLatentActionProxyBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Utilities|FlowControl",
		meta = (WorldContext = "WorldContextObject", HidePin = "UUID,InputCount", BlueprintInternalUseOnly = "true"))
	static UExWaitBranchProxy* CreateProxy_WaitAll(UObject* WorldContextObject, FString UUID, int32 InputCount);

	UFUNCTION(BlueprintCallable, Category = "Utilities|FlowControl",
		meta = (WorldContext = "WorldContextObject", HidePin = "UUID,InputCount", BlueprintInternalUseOnly = "true"))
	static UExWaitBranchProxy* CreateProxy_WaitAny(UObject* WorldContextObject, FString UUID, int32 InputCount);

	UFUNCTION(BlueprintCallable, Category = "Utilities|FlowControl",
		meta = (WorldContext = "WorldContextObject", HidePin = "UUID,InputCount", BlueprintInternalUseOnly = "true"))
	static UExWaitBranchProxy* CreateProxy_WaitCount(UObject* WorldContextObject, FString UUID, int32 InputCount,
		UPARAM(DisplayName = "Required Success Count") int32 RequiredSuccessCount = 1);

protected:
	virtual bool IsFinishAfterBranches() const override { return true; }

	virtual void HandleBranchReported(bool bSuccess) override;

	virtual void OnBranchesFinished() override;

private:
	void InitializeForRun(EExWaitBranchCompletionMode InMode, int32 InRequiredSuccess, int32 ExpectedBranches);

	UPROPERTY()
	EExWaitBranchCompletionMode CompletionMode = EExWaitBranchCompletionMode::All;

	/** Count 模式下需要的最少成功分支数 */
	UPROPERTY()
	int32 RequiredSuccessCount = 1;
	
	int32 ReportsReceived = 0;
	int32 SuccessReceived = 0;
};
