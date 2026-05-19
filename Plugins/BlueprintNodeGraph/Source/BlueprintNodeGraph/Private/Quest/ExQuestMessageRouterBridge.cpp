// Copyright Epic Games, Inc. All Rights Reserved.

#include "Quest/ExQuestMessageRouterBridge.h"

#include "Quest/ExQuestReplicationComponent.h"

void UExQuestMessageRouterBridge::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	RegisterListeners();
}

void UExQuestMessageRouterBridge::Deinitialize()
{
	UnregisterListeners();
	Super::Deinitialize();
}

void UExQuestMessageRouterBridge::RegisterListeners()
{
	UnregisterListeners();

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	const FGameplayTag Channel = ExQuestMessageTags::GetObjectiveProgressChannel();
	if (!Channel.IsValid())
	{
		return;
	}

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GameInstance);
	ObjectiveProgressListenerHandle = MessageSubsystem.RegisterListener<FExQuestObjectiveProgressMessage>(
		Channel,
		this,
		&UExQuestMessageRouterBridge::HandleObjectiveProgress);
}

void UExQuestMessageRouterBridge::UnregisterListeners()
{
	if (ObjectiveProgressListenerHandle.IsValid())
	{
		ObjectiveProgressListenerHandle.Unregister();
		ObjectiveProgressListenerHandle = FGameplayMessageListenerHandle();
	}
}

void UExQuestMessageRouterBridge::HandleObjectiveProgress(FGameplayTag Channel, const FExQuestObjectiveProgressMessage& Message)
{
	UExQuestReplicationComponent::RouteApplyObjectiveProgressMessage(this, Message);
}
