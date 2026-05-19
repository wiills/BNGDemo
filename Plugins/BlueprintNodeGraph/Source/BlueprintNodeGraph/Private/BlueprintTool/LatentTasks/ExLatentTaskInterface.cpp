// Fill out your copyright notice in the Description page of Project Settings.

#include "BlueprintTool/LatentTasks/ExLatentTaskInterface.h"


FString IExLatentTaskInterface::GetStateName() const
{
	return ExLatentTaskHelper::EnumToString(GetState());
}

void IExLatentTaskInterface::TryReset()
{
	SetState(EExLatentTaskState::Pending);
}

void IExLatentTaskInterface::TryStart()
{
	if (GetState() == EExLatentTaskState::Pending)
	{
		SetState(EExLatentTaskState::Running);
		PreOnStart();
		OnStart();
	}
}

void IExLatentTaskInterface::TryStop()
{
	if (GetState() == EExLatentTaskState::Running || GetState() == EExLatentTaskState::Pending)
	{
		SetState(EExLatentTaskState::Completed);
		OnStop();
	}
}

void IExLatentTaskInterface::Terminate()
{
	SetState(EExLatentTaskState::Cancelled);
	if (UObject* Object = Cast<UObject>(this))
	{
		if (IsValid(Object) && !Object->IsUnreachable())
		{
			Object->MarkAsGarbage();
		}
	}
}
