// Copyright Epic Games, Inc. All Rights Reserved.

#include "Quest/ExQuestMessageRouterBridge.h"

#include "Quest/ExQuestReplicationComponent.h"

#if WITH_QUEST_MESSAGE_ROUTER
#include "GameFramework/GameplayMessageSubsystem.h"

namespace ExQuestMessageRouterBridgePrivate
{
	TMap<TWeakObjectPtr<UExQuestMessageRouterBridge>, FGameplayMessageListenerHandle> ListenerHandles;

	void UnregisterListeners(UExQuestMessageRouterBridge* Bridge);

	void RegisterListeners(UExQuestMessageRouterBridge* Bridge)
	{
		if (!Bridge)
		{
			return;
		}

		UnregisterListeners(Bridge);

		UGameInstance* GameInstance = Bridge->GetGameInstance();
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
		FGameplayMessageListenerHandle Handle = MessageSubsystem.RegisterListener<FExQuestMessagePayload>(
			Channel,
			Bridge,
			&UExQuestMessageRouterBridge::HandleQuestMessage);
		ListenerHandles.Add(Bridge, Handle);
	}

	void UnregisterListeners(UExQuestMessageRouterBridge* Bridge)
	{
		if (!Bridge)
		{
			return;
		}

		if (FGameplayMessageListenerHandle* Handle = ListenerHandles.Find(Bridge))
		{
			if (Handle->IsValid())
			{
				Handle->Unregister();
			}
			ListenerHandles.Remove(Bridge);
		}
	}
}
#endif // WITH_QUEST_MESSAGE_ROUTER

void UExQuestMessageRouterBridge::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
#if WITH_QUEST_MESSAGE_ROUTER
	ExQuestMessageRouterBridgePrivate::RegisterListeners(this);
#endif
}

void UExQuestMessageRouterBridge::Deinitialize()
{
#if WITH_QUEST_MESSAGE_ROUTER
	ExQuestMessageRouterBridgePrivate::UnregisterListeners(this);
#endif
	Super::Deinitialize();
}

void UExQuestMessageRouterBridge::HandleQuestMessage(FGameplayTag Channel, const FExQuestMessagePayload& Message)
{
#if WITH_QUEST_MESSAGE_ROUTER
	if (Message.MessageType == EExQuestMessageType::ObjectiveProgress)
	{
		UExQuestReplicationComponent::RouteApplyQuestMessage(this, Message);
	}
#else
	(void)Channel;
	(void)Message;
#endif
}
