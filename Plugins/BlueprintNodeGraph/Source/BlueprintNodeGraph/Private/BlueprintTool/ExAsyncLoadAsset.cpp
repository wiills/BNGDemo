// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/ExAsyncLoadAsset.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/StreamableManager.h"
#include "Engine/World.h"
#include "UObject/UObjectHash.h"
#include "UObject/Package.h"

UExAsyncLoadAsset* UExAsyncLoadAsset::AsyncLoadAssetByPath(
	UObject* WorldContextObject,
	FSoftObjectPath InAssetPath)
{
	UExAsyncLoadAsset* Proxy = NewObject<UExAsyncLoadAsset>();
	Proxy->m_NodeInfo.UUID = FGuid::NewGuid().ToString();
	Proxy->m_NodeInfo.UniqueId = Proxy->m_NodeInfo.UUID;
	
	Proxy->TargetAssetPath = InAssetPath;
	Proxy->bIsClassLoading = false;
	
	Proxy->RegisterWithGameInstance(WorldContextObject);
	return Proxy;
}

UExAsyncLoadAsset* UExAsyncLoadAsset::AsyncLoadAssetClass(
	UObject* WorldContextObject,
	FSoftObjectPath InAssetPath,
	TSubclassOf<UObject> InAssetClass)
{
	UExAsyncLoadAsset* Proxy = NewObject<UExAsyncLoadAsset>();
	Proxy->m_NodeInfo.UUID = FGuid::NewGuid().ToString();
	Proxy->m_NodeInfo.UniqueId = Proxy->m_NodeInfo.UUID;
	
	Proxy->TargetAssetPath = InAssetPath;
	Proxy->TargetAssetClass = InAssetClass;
	Proxy->bIsClassLoading = true;
	
	Proxy->RegisterWithGameInstance(WorldContextObject);
	return Proxy;
}

void UExAsyncLoadAsset::Activate()
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
		FStreamableDelegate::CreateUObject(this, &UExAsyncLoadAsset::HandleAssetLoaded));
}

void UExAsyncLoadAsset::HandleAssetLoaded()
{
	OnAssetLoaded.Broadcast();
	SetReadyToDestroy();
}

void UExAsyncLoadAssetCallbackProxy::OnAssetLoaded(FPrimaryAssetId LoadedId)
{
	if (ParentProxy)
	{
		ParentProxy->HandleAssetLoaded();
	}
}
