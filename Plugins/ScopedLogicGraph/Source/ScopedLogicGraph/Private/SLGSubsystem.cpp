// Copyright Epic Games, Inc. All Rights Reserved.

#include "SLGSubsystem.h"
#include "SLGBakedGraph.h"
#include "SLGInstanceData.h"
#include "SLGExecContext.h"
#include "SLGNode.h"
#include "SLGRunnerComponent.h"
#include "ScopedMessageSubsystem.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"

DEFINE_LOG_CATEGORY(LogSLG);

USLGSubsystem* USLGSubsystem::GetInstance(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr)
	{
		return GameInstance->GetSubsystem<USLGSubsystem>();
	}
	return nullptr;
}

void USLGSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	// 局部寻址依赖消息子系统的 scope 解析，确保其先初始化。
	Collection.InitializeDependency<UScopedMessageSubsystem>();
}

void USLGSubsystem::Deinitialize()
{
	ActiveRunners.Reset();
	ScopeActorRegistry.Reset();
	Super::Deinitialize();
}

AActor* USLGSubsystem::ResolveActorInScope(FScopedMessageScopeId Scope, FGameplayTag RoleTag) const
{
	if (const TMap<FGameplayTag, TWeakObjectPtr<AActor>>* RoleMap = ScopeActorRegistry.Find(Scope))
	{
		if (const TWeakObjectPtr<AActor>* Found = RoleMap->Find(RoleTag))
		{
			return Found->Get();
		}
	}
	return nullptr;
}

void USLGSubsystem::RegisterActorRole(AActor* Actor, FGameplayTag RoleTag)
{
	if (!Actor || !RoleTag.IsValid())
	{
		return;
	}

	UScopedMessageSubsystem* MsgSub = UScopedMessageSubsystem::GetInstance(Actor);
	if (!MsgSub)
	{
		UE_LOG(LogSLG, Warning, TEXT("RegisterActorRole: no message subsystem to resolve scope for '%s'."), *Actor->GetName());
		return;
	}

	const FScopedMessageScopeId Scope = MsgSub->ResolveScopeId(Actor);
	if (!Scope.IsValid())
	{
		UE_LOG(LogSLG, Warning, TEXT("RegisterActorRole: could not resolve scope for '%s'."), *Actor->GetName());
		return;
	}

	ScopeActorRegistry.FindOrAdd(Scope).Add(RoleTag, Actor);
}

void USLGSubsystem::UnregisterActorRole(AActor* Actor, FGameplayTag RoleTag)
{
	if (!Actor)
	{
		return;
	}

	if (UScopedMessageSubsystem* MsgSub = UScopedMessageSubsystem::GetInstance(Actor))
	{
		const FScopedMessageScopeId Scope = MsgSub->ResolveScopeId(Actor);
		if (TMap<FGameplayTag, TWeakObjectPtr<AActor>>* RoleMap = ScopeActorRegistry.Find(Scope))
		{
			RoleMap->Remove(RoleTag);
			if (RoleMap->Num() == 0)
			{
				ScopeActorRegistry.Remove(Scope);
			}
		}
	}
}

void USLGSubsystem::RegisterRunner(USLGRunnerComponent* Runner)
{
	if (Runner)
	{
		ActiveRunners.AddUnique(Runner);
	}
}

void USLGSubsystem::UnregisterRunner(USLGRunnerComponent* Runner)
{
	ActiveRunners.RemoveAll([Runner](const TWeakObjectPtr<USLGRunnerComponent>& Weak)
	{
		return !Weak.IsValid() || Weak.Get() == Runner;
	});
}

void USLGSubsystem::ExecuteGraphSerial(
	FScopedMessageScopeId ScopeId,
	const FSLGBakedGraph& Baked,
	FSLGInstanceData& Instance,
	UWorld* World)
{
	if (!Baked.IsValidBaked())
	{
		return;
	}

	// M0：全 GameThread，按拓扑序逐节点跑一遍。并行（worker/分帧）后置 M2。
	FSLGExecContext Ctx;
	Ctx.ScopeId = ScopeId;
	Ctx.Baked = &Baked;
	Ctx.Instance = &Instance;
	Ctx.Subsystem = this;
	Ctx.World = World;

	for (int32 NodeIndex : Baked.ExecutionOrder)
	{
		if (!Baked.Nodes.IsValidIndex(NodeIndex))
		{
			continue;
		}

		const FSLGNodeBase* Node = Baked.Nodes[NodeIndex].NodeConfig.GetPtr<FSLGNodeBase>();
		if (!Node)
		{
			continue;
		}

		Ctx.CurrentNodeIndex = NodeIndex;
		Node->Execute(Ctx);
	}
}
