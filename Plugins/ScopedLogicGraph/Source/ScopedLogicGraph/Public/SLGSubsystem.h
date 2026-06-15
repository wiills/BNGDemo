// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ScopedMessageTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SLGSubsystem.generated.h"

struct FSLGBakedGraph;
struct FSLGInstanceData;
class USLGRunnerComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogSLG, Log, All);

/**
 * SLG 全局基础设施：图实例表 + 局部寻址反查 + 调度入口。
 * GameInstance subsystem owning the registry of active graph instances, the reverse
 * (ScopeId, RoleTag) -> actor lookup, and the per-frame scheduling entry point.
 *
 * 局部寻址：现有 ScopedMessageSubsystem 已有 ResolveScopeId(object -> scopeId)；
 * 本子系统只补反向查询，杀死"全局唯一 ID"。
 */
UCLASS(DisplayName = "Scoped Logic Graph Subsystem")
class SCOPEDLOGICGRAPH_API USLGSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static USLGSubsystem* GetInstance(const UObject* WorldContextObject);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ---- 局部寻址 ScopeActorRegistry ----------------------------------------

	/**
	 * 在指定作用域内按角色标签解析 actor。
	 * (ScopeId, RoleTag) -> actor inside that scope. No global unique IDs needed.
	 */
	UFUNCTION(BlueprintCallable, Category = "SLG|Addressing")
	AActor* ResolveActorInScope(FScopedMessageScopeId Scope, FGameplayTag RoleTag) const;

	/** 登记某 actor 在其所属作用域内的角色（scope 内部解析）。 */
	UFUNCTION(BlueprintCallable, Category = "SLG|Addressing")
	void RegisterActorRole(AActor* Actor, FGameplayTag RoleTag);

	/** 注销某 actor 的角色登记。 */
	UFUNCTION(BlueprintCallable, Category = "SLG|Addressing")
	void UnregisterActorRole(AActor* Actor, FGameplayTag RoleTag);

	// ---- 运行器登记与调度 ----------------------------------------------------

	/** 运行器激活时登记，参与全局调度。 */
	void RegisterRunner(USLGRunnerComponent* Runner);

	/** 运行器休眠/销毁时注销。 */
	void UnregisterRunner(USLGRunnerComponent* Runner);

	// ---- 执行器（M0：全 GameThread 串行） -----------------------------------

	/**
	 * 按烘焙态执行序串行跑一遍图（M0 正确性优先，并行后置 M2）。
	 * Run one serial pass over the baked graph on GameThread.
	 */
	void ExecuteGraphSerial(
		FScopedMessageScopeId ScopeId,
		const FSLGBakedGraph& Baked,
		FSLGInstanceData& Instance,
		UWorld* World);

private:
	/** 已激活的运行器（弱引用，避免阻止 GC）。 */
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<USLGRunnerComponent>> ActiveRunners;

	/** ScopeId -> (RoleTag -> actor)。局部寻址核心表（唯一新建基础设施）。 */
	TMap<FScopedMessageScopeId, TMap<FGameplayTag, TWeakObjectPtr<AActor>>> ScopeActorRegistry;
};
