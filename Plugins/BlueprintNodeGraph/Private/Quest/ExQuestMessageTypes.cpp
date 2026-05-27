// Copyright Epic Games, Inc. All Rights Reserved.

#include "Quest/ExQuestMessageTypes.h"

#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Quest_Event_Objective_Progress, "Quest.Event.Objective.Progress");

FGameplayTag ExQuestMessageTags::GetObjectiveProgressChannel()
{
	return TAG_Quest_Event_Objective_Progress;
}

FGameplayTag ExQuestMessageTags::GetChannelForMessageType(EExQuestMessageType MessageType)
{
	switch (MessageType)
	{
	case EExQuestMessageType::ObjectiveProgress:
		return GetObjectiveProgressChannel();
	default:
		return FGameplayTag();
	}
}
