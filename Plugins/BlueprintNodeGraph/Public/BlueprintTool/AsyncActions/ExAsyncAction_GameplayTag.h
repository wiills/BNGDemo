// Copyright BlueprintNodeGraph. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/AsyncActions/ExBase_AsyncAction.h"
#include "GameplayTagContainer.h"
#include "ExAsyncAction_GameplayTag.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameplayTagChangedDelegate, FGameplayTag, Tag);

UCLASS()
class BLUEPRINTNODEGRAPH_API UExAsyncAction_GameplayTagListener : public UExBase_AsyncAction
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "GameplayTag")
	FOnGameplayTagChangedDelegate OnTagAdded;

	UPROPERTY(BlueprintAssignable, Category = "GameplayTag")
	FOnGameplayTagChangedDelegate OnTagRemoved;

protected:
	UPROPERTY()
	FGameplayTag TagToListen;

public:
	UFUNCTION(BlueprintCallable, Category = "GameplayTag", meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Listen for Gameplay Tag"))
	static UExAsyncAction_GameplayTagListener* CreateProxy_ListenTag(UObject* WorldContextObject, FGameplayTag Tag);

	virtual void Activate() override;
	void HandleTagChange(const FGameplayTag CallbackTag, int32 NewCount);
};

UCLASS()
class BLUEPRINTNODEGRAPH_API UExAsyncAction_GameplayTagQuery : public UExBase_AsyncAction
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "GameplayTag")
	FOnGameplayTagChangedDelegate OnQueryResult;

protected:
	UPROPERTY()
	FGameplayTagContainer TagContainer;

public:
	UFUNCTION(BlueprintCallable, Category = "GameplayTag", meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Query Gameplay Tags"))
	static UExAsyncAction_GameplayTagQuery* CreateProxy_QueryTags(UObject* WorldContextObject, FGameplayTagContainer QueryTags);

	UFUNCTION(BlueprintCallable, Category = "GameplayTag", meta = (BlueprintInternalUseOnly = "true"))
	void AddTag(FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category = "GameplayTag", meta = (BlueprintInternalUseOnly = "true"))
	void RemoveTag(FGameplayTag Tag);

	virtual void Activate() override;
};

UCLASS()
class BLUEPRINTNODEGRAPH_API UExAsyncAction_GameplayTagModifier : public UExBase_AsyncAction
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "GameplayTag")
	FOnGameplayTagChangedDelegate OnTagsModified;

protected:
	UPROPERTY()
	FGameplayTagContainer TagsToAdd;

	UPROPERTY()
	FGameplayTagContainer TagsToRemove;

public:
	UFUNCTION(BlueprintCallable, Category = "GameplayTag", meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Add Gameplay Tags"))
	static UExAsyncAction_GameplayTagModifier* CreateProxy_AddTags(UObject* WorldContextObject, FGameplayTagContainer Tags);

	UFUNCTION(BlueprintCallable, Category = "GameplayTag", meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Remove Gameplay Tags"))
	static UExAsyncAction_GameplayTagModifier* CreateProxy_RemoveTags(UObject* WorldContextObject, FGameplayTagContainer Tags);

	UFUNCTION(BlueprintCallable, Category = "GameplayTag", meta = (BlueprintInternalUseOnly = "true"))
	void AddTagToAdd(FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category = "GameplayTag", meta = (BlueprintInternalUseOnly = "true"))
	void AddTagToRemove(FGameplayTag Tag);

	virtual void Activate() override;
};
