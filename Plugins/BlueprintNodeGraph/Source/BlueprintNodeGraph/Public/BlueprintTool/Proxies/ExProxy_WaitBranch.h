// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/Subsystems/ExLatentActionManager.h"
#include "BlueprintTool/Common/ExBranchMode.h"
#include "ExProxy_WaitBranch.generated.h"

/**
 * Wait Multi-Input Proxy
 * All（全部输入已报告）/ Any（任一路成功）/ Count（成功路数达到阈值）；单次输入对应一次 Activate / ReportBranchFailed。
 * 每条输入执行引脚默认调用 Activate() 视为成功；若需将某路记为失败请在该支调用 ReportBranchFailed()。
 */
UCLASS()
class BLUEPRINTNODEGRAPH_API UExProxy_WaitBranch : public UExBase_FlowProxy
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Utilities|FlowControl",
		meta = (WorldContext = "WorldContextObject", HidePin = "UUID,InputCount", BlueprintInternalUseOnly = "true"))
	static UExProxy_WaitBranch* CreateProxy_WaitAll(UObject* WorldContextObject, FString UUID, int32 InputCount);

	UFUNCTION(BlueprintCallable, Category = "Utilities|FlowControl",
		meta = (WorldContext = "WorldContextObject", HidePin = "UUID,InputCount", BlueprintInternalUseOnly = "true"))
	static UExProxy_WaitBranch* CreateProxy_WaitAny(UObject* WorldContextObject, FString UUID, int32 InputCount);

	UFUNCTION(BlueprintCallable, Category = "Utilities|FlowControl",
		meta = (WorldContext = "WorldContextObject", HidePin = "UUID,InputCount", BlueprintInternalUseOnly = "true"))
	static UExProxy_WaitBranch* CreateProxy_WaitCount(UObject* WorldContextObject, FString UUID, int32 InputCount,
		UPARAM(DisplayName = "Required Success Count") int32 RequiredSuccessCount = 1);

protected:
	virtual bool IsRemoveAfterBranches() const override { return false; }

	virtual void OnOneBranchFinished() override;

	virtual void OnBranchesFinished() override;

private:
	void InitializeForRun(EExBranchMode InMode, int32 InRequiredSuccess);

	UPROPERTY()
	EExBranchMode BranchMode = EExBranchMode::All;
};
