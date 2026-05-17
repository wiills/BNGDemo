// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintTool/ExWaitBranchProxy.h"
#include "BlueprintTool/ExLatentActionManager.h"


UExWaitBranchProxy* UExWaitBranchProxy::CreateProxy(UObject* WorldContextObject, FString UUID, int32 InputCount)
{
	return CreateWaitProxyCall<UExWaitBranchProxy>(WorldContextObject, UUID, InputCount);
}

void UExWaitBranchProxy::OnBranchesFinished()
{
	UE_LOG(LogAsyncAction, Display, TEXT("[UExWaitBranchProxy::OnBranchesFinished] - %s"), *GetName());
}
