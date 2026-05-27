// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/AsyncActions/ExAsyncAction_LoadAsset.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

UExAsyncAction_LoadAsset* UExAsyncAction_LoadAsset::AsyncLoadAssetByPath(
	UObject* WorldContextObject,
	FSoftObjectPath InAssetPath)
{
	UExAsyncAction_LoadAsset* Proxy = NewObject<UExAsyncAction_LoadAsset>();
	Proxy->m_NodeInfo.UUID = FGuid::NewGuid().ToString();
	Proxy->m_NodeInfo.UniqueId = Proxy->m_NodeInfo.UUID;
	
	Proxy->TargetAssetPath = InAssetPath;
	Proxy->bIsClassLoading = false;
	
	Proxy->RegisterWithGameInstance(WorldContextObject);
	return Proxy;
}

UExAsyncAction_LoadAsset* UExAsyncAction_LoadAsset::AsyncLoadAssetClass(
	UObject* WorldContextObject,
	FSoftObjectPath InAssetPath,
	TSubclassOf<UObject> InAssetClass)
{
	UExAsyncAction_LoadAsset* Proxy = NewObject<UExAsyncAction_LoadAsset>();
	Proxy->m_NodeInfo.UUID = FGuid::NewGuid().ToString();
	Proxy->m_NodeInfo.UniqueId = Proxy->m_NodeInfo.UUID;
	
	Proxy->TargetAssetPath = InAssetPath;
	Proxy->TargetAssetClass = InAssetClass;
	Proxy->bIsClassLoading = true;
	
	Proxy->RegisterWithGameInstance(WorldContextObject);
	return Proxy;
}

void UExAsyncAction_LoadAsset::Activate()
{
	Super::Activate();
	
	if (TargetAssetPath.IsNull())
	{
		HandleAssetLoaded();
		return;
	}

	if (!bIsClassLoading)
	{
		if (TargetAssetPath.ResolveObject())
		{
			HandleAssetLoaded();
			return;
		}
	}
	else
	{
		if (UObject* Resolved = TargetAssetPath.ResolveObject())
		{
			if (UClass* AsClass = Cast<UClass>(Resolved))
			{
				if (TargetAssetClass && AsClass->IsChildOf(TargetAssetClass.Get()))
				{
					HandleAssetLoaded();
					return;
				}
			}
		}
	}

	UAssetManager::GetStreamableManager().RequestAsyncLoad(
		TargetAssetPath,
		FStreamableDelegate::CreateUObject(this, &UExAsyncAction_LoadAsset::HandleAssetLoaded));
}

void UExAsyncAction_LoadAsset::HandleAssetLoaded()
{
	OnAssetLoaded.Broadcast();
	SetReadyToDestroy();
}

void UExAsyncAction_LoadAssetCallbackProxy::OnAssetLoaded(FPrimaryAssetId LoadedId)
{
	if (ParentProxy)
	{
		ParentProxy->HandleAssetLoaded();
	}
}
