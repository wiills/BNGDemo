// Copyright BlueprintNodeGraph. All Rights Reserved.

#include "BlueprintTool/AsyncActions/ExAsyncAction_GameplayTag.h"

UExAsyncAction_GameplayTagListener* UExAsyncAction_GameplayTagListener::CreateProxy_ListenTag(UObject* WorldContextObject, FGameplayTag Tag)
{
	auto* Proxy = NewObject<UExAsyncAction_GameplayTagListener>();
	Proxy->TagToListen = Tag;
	Proxy->RegisterWithGameInstance(WorldContextObject);
	return Proxy;
}

void UExAsyncAction_GameplayTagListener::Activate()
{
	Super::Activate();
	
	UWorld* World = GetWorld();
	if (!World)
	{
		SetReadyToDestroy();
		return;
	}

	UE_LOG(LogAsyncAction, Display, TEXT("[UExAsyncAction_GameplayTagListener] Listening for tag: %s"), *TagToListen.ToString());
}

void UExAsyncAction_GameplayTagListener::HandleTagChange(const FGameplayTag CallbackTag, int32 NewCount)
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

UExAsyncAction_GameplayTagQuery* UExAsyncAction_GameplayTagQuery::CreateProxy_QueryTags(UObject* WorldContextObject, FGameplayTagContainer QueryTags)
{
	auto* Proxy = NewObject<UExAsyncAction_GameplayTagQuery>();
	Proxy->TagContainer = QueryTags;
	Proxy->RegisterWithGameInstance(WorldContextObject);
	return Proxy;
}

void UExAsyncAction_GameplayTagQuery::AddTag(FGameplayTag Tag)
{
	TagContainer.AddTag(Tag);
}

void UExAsyncAction_GameplayTagQuery::RemoveTag(FGameplayTag Tag)
{
	TagContainer.RemoveTag(Tag);
}

void UExAsyncAction_GameplayTagQuery::Activate()
{
	Super::Activate();
	
	for (const FGameplayTag& Tag : TagContainer)
	{
		OnQueryResult.Broadcast(Tag);
	}
	
	SetReadyToDestroy();
}

UExAsyncAction_GameplayTagModifier* UExAsyncAction_GameplayTagModifier::CreateProxy_AddTags(UObject* WorldContextObject, FGameplayTagContainer Tags)
{
	auto* Proxy = NewObject<UExAsyncAction_GameplayTagModifier>();
	Proxy->TagsToAdd = Tags;
	Proxy->RegisterWithGameInstance(WorldContextObject);
	return Proxy;
}

UExAsyncAction_GameplayTagModifier* UExAsyncAction_GameplayTagModifier::CreateProxy_RemoveTags(UObject* WorldContextObject, FGameplayTagContainer Tags)
{
	auto* Proxy = NewObject<UExAsyncAction_GameplayTagModifier>();
	Proxy->TagsToRemove = Tags;
	Proxy->RegisterWithGameInstance(WorldContextObject);
	return Proxy;
}

void UExAsyncAction_GameplayTagModifier::AddTagToAdd(FGameplayTag Tag)
{
	TagsToAdd.AddTag(Tag);
}

void UExAsyncAction_GameplayTagModifier::AddTagToRemove(FGameplayTag Tag)
{
	TagsToRemove.AddTag(Tag);
}

void UExAsyncAction_GameplayTagModifier::Activate()
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
