// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/LatentTasks/ExLatentTask_Custom.h"

#include "BlueprintTool/Subsystems/ExLatentActionManager.h"
#include "Kismet/GameplayStatics.h"

UExLatentTask_Custom* UExLatentTask_Custom::CreateProxy(UObject* WorldContextObject, TSubclassOf<UExLatentTask_Custom> Class)
{
	return Cast<UExLatentTask_Custom>(UGameplayStatics::SpawnObject(Class, WorldContextObject));
}

UExLatentTask_BranchSync* UExLatentTask_BranchSync::CreateProxy(UObject* WorldContextObject,
	TSubclassOf<UExLatentTask_BranchSync> Class, FString UUID, int32 InputCount)
{
	return CreateWaitProxyCallWithClass<UExLatentTask_BranchSync>(WorldContextObject, Class, UUID, InputCount);
}

void UExLatentTask_BranchSync::OnStop()
{
	RemoveWaitInstance();
	Super::OnStop();
}

void UExLatentTask_BranchSync::RemoveWaitInstance() const
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
