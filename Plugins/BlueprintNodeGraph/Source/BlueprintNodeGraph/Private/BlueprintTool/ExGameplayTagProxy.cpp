// Copyright BlueprintNodeGraph. All Rights Reserved.

#include "BlueprintTool/ExGameplayTagProxy.h"

UExGameplayTagListenerProxy* UExGameplayTagListenerProxy::CreateProxy_ListenTag(UObject* WorldContextObject, FGameplayTag Tag)
{
	auto* Proxy = NewObject<UExGameplayTagListenerProxy>();
	Proxy->TagToListen = Tag;
	Proxy->RegisterWithGameInstance(WorldContextObject);
	return Proxy;
}

void UExGameplayTagListenerProxy::Activate()
{
	Super::Activate();
	
	UWorld* World = GetWorld();
	if (!World)
	{
		SetReadyToDestroy();
		return;
	}

	UE_LOG(LogAsyncAction, Display, TEXT("[UExGameplayTagListenerProxy] Listening for tag: %s"), *TagToListen.ToString());
}

void UExGameplayTagListenerProxy::HandleTagChange(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (CallbackTag.MatchesTag(TagToListen))
	{
		if (NewCount > 0)
		{
			OnTagAdded.Broadcast(CallbackTag);
		}
		else
		{
			OnTagRemoved.Broadcast(CallbackTag);
		}
		SetReadyToDestroy();
	}
}

UExGameplayTagQueryProxy* UExGameplayTagQueryProxy::CreateProxy_QueryTags(UObject* WorldContextObject, FGameplayTagContainer QueryTags)
{
	auto* Proxy = NewObject<UExGameplayTagQueryProxy>();
	Proxy->TagContainer = QueryTags;
	Proxy->RegisterWithGameInstance(WorldContextObject);
	return Proxy;
}

void UExGameplayTagQueryProxy::AddTag(FGameplayTag Tag)
{
	TagContainer.AddTag(Tag);
}

void UExGameplayTagQueryProxy::RemoveTag(FGameplayTag Tag)
{
	TagContainer.RemoveTag(Tag);
}

void UExGameplayTagQueryProxy::Activate()
{
	Super::Activate();
	
	for (const FGameplayTag& Tag : TagContainer)
	{
		OnQueryResult.Broadcast(Tag);
	}
	
	SetReadyToDestroy();
}

UExGameplayTagModifierProxy* UExGameplayTagModifierProxy::CreateProxy_AddTags(UObject* WorldContextObject, FGameplayTagContainer Tags)
{
	auto* Proxy = NewObject<UExGameplayTagModifierProxy>();
	Proxy->TagsToAdd = Tags;
	Proxy->RegisterWithGameInstance(WorldContextObject);
	return Proxy;
}

UExGameplayTagModifierProxy* UExGameplayTagModifierProxy::CreateProxy_RemoveTags(UObject* WorldContextObject, FGameplayTagContainer Tags)
{
	auto* Proxy = NewObject<UExGameplayTagModifierProxy>();
	Proxy->TagsToRemove = Tags;
	Proxy->RegisterWithGameInstance(WorldContextObject);
	return Proxy;
}

void UExGameplayTagModifierProxy::AddTagToAdd(FGameplayTag Tag)
{
	TagsToAdd.AddTag(Tag);
}

void UExGameplayTagModifierProxy::AddTagToRemove(FGameplayTag Tag)
{
	TagsToRemove.AddTag(Tag);
}

void UExGameplayTagModifierProxy::Activate()
{
	Super::Activate();
	
	for (const FGameplayTag& Tag : TagsToAdd)
	{
		OnTagsModified.Broadcast(Tag);
	}
	
	for (const FGameplayTag& Tag : TagsToRemove)
	{
		OnTagsModified.Broadcast(Tag);
	}
	
	SetReadyToDestroy();
}
