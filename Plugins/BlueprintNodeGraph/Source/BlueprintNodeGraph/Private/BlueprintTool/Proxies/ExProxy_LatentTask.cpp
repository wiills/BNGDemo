// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/Proxies/ExProxy_LatentTask.h"

#include "BlueprintTool/Subsystems/ExLatentActionManager.h"
#include "Kismet/GameplayStatics.h"

UExProxy_LatentTask* UExProxy_LatentTask::CreateProxy(UObject* WorldContextObject, TSubclassOf<UExProxy_LatentTask> Class)
{
	return Cast<UExProxy_LatentTask>(UGameplayStatics::SpawnObject(Class, WorldContextObject));
}

UExProxy_LatentTaskUUID* UExProxy_LatentTaskUUID::CreateProxy(UObject* WorldContextObject,
	TSubclassOf<UExProxy_LatentTaskUUID> Class, FString UUID, int32 InputCount)
{
	return CreateWaitProxyCallWithClass<UExProxy_LatentTaskUUID>(WorldContextObject, Class, UUID, InputCount);
}

void UExProxy_LatentTaskUUID::OnStop()
{
	RemoveWaitInstance();
	Super::OnStop();
}

void UExProxy_LatentTaskUUID::RemoveWaitInstance() const
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
