// Copyright Epic Games, Inc. All Rights Reserved.

#include "SLGRunnerComponent.h"
#include "SLGGraphAsset.h"
#include "SLGSubsystem.h"
#include "SLGBakedGraph.h"
#include "ScopedMessageSubsystem.h"
#include "ScopedMessageScopeComponent.h"

USLGRunnerComponent::USLGRunnerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bAutoActivate = false;
}

void USLGRunnerComponent::BeginPlay()
{
	Super::BeginPlay();

	Subsystem = USLGSubsystem::GetInstance(this);
	ResolveScopeId();

	// 生命周期复用 ScopedMessageSystem 玩家兴趣机制；此处仅做骨架级直接激活。
	// 实际激活时机由"POI 是否有人"驱动（M1 接玩家进出回调）。
	Activate(true);
}

void USLGRunnerComponent::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Deactivate();
	Super::EndPlay(EndPlayReason);
}

void USLGRunnerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (InstanceState != ESLGInstanceState::Active || !GraphAsset)
	{
		return;
	}

	if (USLGSubsystem* Sub = Subsystem.Get())
	{
		Sub->ExecuteGraphSerial(ScopeId, GraphAsset->Baked, InstanceData, GetWorld());
	}
}

void USLGRunnerComponent::Activate(bool bReset)
{
	Super::Activate(bReset);

	if (!GraphAsset)
	{
		UE_LOG(LogSLG, Warning, TEXT("SLGRunner on '%s' has no GraphAsset."), *GetNameSafe(GetOwner()));
		return;
	}

	// 惰性烘焙（待定问题 #5：M0 取加载/激活时烘焙）。
	if (!GraphAsset->IsBaked() && !GraphAsset->Bake())
	{
		UE_LOG(LogSLG, Error, TEXT("SLGRunner: bake failed for '%s'."), *GetNameSafe(GraphAsset));
		return;
	}

	if (bReset || !InstanceData.bInitialized)
	{
		AllocateInstanceData();
	}

	InstanceState = ESLGInstanceState::Active;
	SetComponentTickEnabled(true);

	if (USLGSubsystem* Sub = Subsystem.Get())
	{
		Sub->RegisterRunner(this);
	}
}

void USLGRunnerComponent::Deactivate()
{
	Super::Deactivate();

	SetComponentTickEnabled(false);

	if (USLGSubsystem* Sub = Subsystem.Get())
	{
		Sub->UnregisterRunner(this);
	}

	// 决策 #2：无人时休眠；此处保留实例数据，彻底释放可在销毁/资源紧张时做。
	InstanceState = ESLGInstanceState::Dormant;
}

void USLGRunnerComponent::ResolveScopeId()
{
	if (UScopedMessageScopeComponent* ScopeComp = GetOwner() ? GetOwner()->FindComponentByClass<UScopedMessageScopeComponent>() : nullptr)
	{
		ScopeId = ScopeComp->GetScopeId_Implementation();
	}
	else if (UScopedMessageSubsystem* MsgSub = UScopedMessageSubsystem::GetInstance(this))
	{
		ScopeId = MsgSub->ResolveScopeId(GetOwner());
	}

	if (!ScopeId.IsValid())
	{
		UE_LOG(LogSLG, Warning, TEXT("SLGRunner on '%s' could not resolve a ScopeId."), *GetNameSafe(GetOwner()));
	}
}

void USLGRunnerComponent::AllocateInstanceData()
{
	ReleaseInstanceData();

	const FSLGBakedGraph& Baked = GraphAsset->Baked;

	// 一次性分配连续 POD blob，再就地构造各有状态节点的实例结构。
	InstanceData.NodeStateBlob.SetNumUninitialized(Baked.InstanceBlobSize);
	for (const FSLGBakedNode& Node : Baked.Nodes)
	{
		if (Node.HasInstanceState() && Node.InstanceType)
		{
			uint8* Slot = InstanceData.NodeStateBlob.GetData() + Node.InstanceOffset;
			Node.InstanceType->InitializeStruct(Slot);
		}
	}

	InstanceData.bInitialized = true;
}

void USLGRunnerComponent::ReleaseInstanceData()
{
	if (!InstanceData.bInitialized)
	{
		InstanceData.Reset();
		return;
	}

	// 析构 blob 中非平凡字段，避免泄漏。
	const FSLGBakedGraph& Baked = GraphAsset->Baked;
	for (const FSLGBakedNode& Node : Baked.Nodes)
	{
		if (Node.HasInstanceState() && Node.InstanceType && InstanceData.NodeStateBlob.IsValidIndex(Node.InstanceOffset))
		{
			uint8* Slot = InstanceData.NodeStateBlob.GetData() + Node.InstanceOffset;
			Node.InstanceType->DestroyStruct(Slot);
		}
	}

	InstanceData.Reset();
}
