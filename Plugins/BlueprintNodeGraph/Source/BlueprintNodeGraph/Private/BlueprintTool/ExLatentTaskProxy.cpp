// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/ExLatentTaskProxy.h"

#include "BlueprintTool/ExLatentActionManager.h"
#include "Kismet/GameplayStatics.h"

UExLatentTaskProxy* UExLatentTaskProxy::CreateProxy(UObject* WorldContextObject, TSubclassOf<UExLatentTaskProxy> Class)
{
	return Cast<UExLatentTaskProxy>(UGameplayStatics::SpawnObject(Class, WorldContextObject));
}

UExLatentTaskUUIDProxy* UExLatentTaskUUIDProxy::CreateProxy(UObject* WorldContextObject,
	TSubclassOf<UExLatentTaskUUIDProxy> Class, FString UUID, int32 InputCount)
{
	return CreateWaitProxyCallWithClass<UExLatentTaskUUIDProxy>(WorldContextObject, Class, UUID, InputCount);
}

void UExLatentTaskUUIDProxy::OnStop()
{
	RemoveWaitInstance();
	Super::OnStop();
}

void UExLatentTaskUUIDProxy::RemoveWaitInstance() const
{
	if (IsBranchesFinished())
	{
		const auto WaitInputManager = GetWorld()->GetGameInstance()->GetSubsystem<UExLatentActionManager>();
		if (WaitInputManager)
		{
			WaitInputManager->RemoveProxyObject(m_SelfUUID);
		}
	}
}
