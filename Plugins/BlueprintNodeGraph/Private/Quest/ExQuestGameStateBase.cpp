// Copyright Epic Games, Inc. All Rights Reserved.

#include "Quest/ExQuestGameStateBase.h"
#include "Quest/ExQuestReplicationComponent.h"

AExQuestGameStateBase::AExQuestGameStateBase()
{
	QuestReplication = CreateDefaultSubobject<UExQuestReplicationComponent>(TEXT("QuestReplication"));
}
