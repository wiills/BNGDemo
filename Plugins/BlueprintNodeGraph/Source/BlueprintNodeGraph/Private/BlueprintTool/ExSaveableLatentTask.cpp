// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/ExSaveableLatentTask.h"
#include "BlueprintTool/ExSaveGameLibrary.h"
#include "BlueprintTool/ExSaveGameTypes.h"
#include "Engine/World.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

FExFlowTaskState UExSaveableLatentTask::SerializeState()
{
	FExFlowTaskState State;
	State.TaskId = NodeInfo.UUID;
	State.TaskClassName = GetClass()->GetName();
	State.CurrentState = ExLatentTaskHelper::EnumToString(GetState());
	State.TaskParameters = GetTaskSpecificData();
	State.StartTimestamp = LastSaveTimestamp;
	State.LastUpdateTimestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	State.bIsCompleted = (GetState() == EExLatentTaskState::Completed);
	State.CompletedTimestamp = State.bIsCompleted ? State.LastUpdateTimestamp : 0.f;
	State.CheckpointIndex = CurrentCheckpointIndex;

	return State;
}

void UExSaveableLatentTask::DeserializeState(const FExFlowTaskState& State)
{
	NodeInfo.UUID = State.TaskId;
	CurrentCheckpointIndex = State.CheckpointIndex;
	LastSaveTimestamp = State.LastUpdateTimestamp;
	
	if (!State.TaskParameters.IsEmpty())
	{
		RestoreTaskSpecificData(State.TaskParameters);
	}
}

FString UExSaveableLatentTask::GetCheckpointData()
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	
	JsonObject->SetStringField(TEXT("TaskId"), NodeInfo.UUID);
	JsonObject->SetStringField(TEXT("TaskClass"), GetClass()->GetName());
	JsonObject->SetStringField(TEXT("State"), ExLatentTaskHelper::EnumToString(GetState()));
	JsonObject->SetNumberField(TEXT("CheckpointIndex"), CurrentCheckpointIndex);
	JsonObject->SetStringField(TEXT("TaskData"), GetTaskSpecificData());
	JsonObject->SetNumberField(TEXT("Timestamp"), GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f);

	FString OutputString;
	TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<TCHAR>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);

	return OutputString;
}

void UExSaveableLatentTask::RestoreFromCheckpoint(const FString& CheckpointData)
{
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(CheckpointData);
	TSharedPtr<FJsonObject> JsonObject;
	
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
	{
		NodeInfo.UUID = JsonObject->GetStringField(TEXT("TaskId"));
		CurrentCheckpointIndex = JsonObject->GetIntegerField(TEXT("CheckpointIndex"));

		FString TaskData = JsonObject->GetStringField(TEXT("TaskData"));
		if (!TaskData.IsEmpty())
		{
			RestoreTaskSpecificData(TaskData);
		}
	}
}

void UExSaveableLatentTask::CheckAutoSave()
{
	if (!bAutoSave)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	float CurrentTime = World->GetTimeSeconds();
	if (CurrentTime - LastSaveTimestamp >= AutoSaveInterval)
	{
		LastSaveTimestamp = CurrentTime;
		UE_LOG(LogLatentTask, Display, TEXT("Auto-save triggered for task: %s"), *NodeInfo.UUID);
	}
}

void UExSaveableLatentTask::MarkCheckpoint()
{
	CurrentCheckpointIndex++;
	UWorld* World = GetWorld();
	LastSaveTimestamp = World ? World->GetTimeSeconds() : 0.f;
	
	UE_LOG(LogLatentTask, Display, TEXT("Checkpoint marked: %d for task: %s"), CurrentCheckpointIndex, *NodeInfo.UUID);
}
