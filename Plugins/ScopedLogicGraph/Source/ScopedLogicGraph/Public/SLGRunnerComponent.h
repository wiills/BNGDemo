// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ScopedMessageTypes.h"
#include "SLGTypes.h"
#include "SLGInstanceData.h"
#include "SLGRunnerComponent.generated.h"

class USLGGraphAsset;
class USLGSubsystem;
class UScopedMessageSubsystem;

/**
 * 图运行器组件：绑定 ScopeId、实例化烘焙态、按 POI 是否有人管理生命周期。
 * Drives one graph instance. Add it to an actor that also carries a
 * UScopedMessageScopeComponent (e.g. AScopedMessagePoiRootActor). The runner pulls the
 * ScopeId from that component, allocates FSLGInstanceData from the baked layout, and
 * registers with USLGSubsystem for scheduling.
 *
 * 生命周期复用 ScopedMessageSystem 的玩家兴趣机制（决策 #2）：POI 有人才激活。
 */
UCLASS(ClassGroup = (SLG), meta = (BlueprintSpawnableComponent))
class SCOPEDLOGICGRAPH_API USLGRunnerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USLGRunnerComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 要运行的图模板。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SLG")
	TObjectPtr<USLGGraphAsset> GraphAsset;

	/** 实例当前生命周期状态。 */
	UFUNCTION(BlueprintPure, Category = "SLG")
	ESLGInstanceState GetInstanceState() const { return InstanceState; }

	/** 取本运行器所属作用域。 */
	UFUNCTION(BlueprintPure, Category = "SLG")
	FScopedMessageScopeId GetScopeId() const { return ScopeId; }

	/** 激活：分配实例数据并向子系统登记（POI 有人时调用）。重写自 UActorComponent。 */
	virtual void Activate(bool bReset = false) override;

	/** 休眠：保留/释放实例数据，停止参与调度（POI 无人时调用）。重写自 UActorComponent。 */
	virtual void Deactivate() override;

	/** 取本实例可变状态（执行器使用）。 */
	FSLGInstanceData& GetInstanceData() { return InstanceData; }

private:
	/** 从同 actor 的 ScopeComponent 解析 ScopeId。 */
	void ResolveScopeId();

	/** 占用变化回调：仅本 Scope 触发时驱动 wake/sleep（决策 #2）。 */
	void HandleScopeOccupancyChanged(FScopedMessageScopeId ChangedScope, int32 PlayerCount);

	/** 按烘焙态布局分配连续 POD 实例状态。 */
	void AllocateInstanceData();

	/** 释放实例状态。 */
	void ReleaseInstanceData();

	UPROPERTY(Transient)
	FScopedMessageScopeId ScopeId;

	UPROPERTY(Transient)
	ESLGInstanceState InstanceState = ESLGInstanceState::Uninitialized;

	/** 本 Scope 一份的运行态实例数据（连续 POD + 黑板）。 */
	FSLGInstanceData InstanceData;

	TWeakObjectPtr<USLGSubsystem> Subsystem;

	/** 消息子系统（占用信号来源），用于解绑委托。 */
	TWeakObjectPtr<UScopedMessageSubsystem> MessageSubsystem;

	/** OnScopeOccupancyChanged 订阅句柄。 */
	FDelegateHandle OccupancyChangedHandle;
};
