// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/ExSaveGameLibrary.h"
#include "BlueprintTool/ExSaveGameTypes.h"
#include "Engine/World.h"
#include "GameFramework/SaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

bool UExSaveGameLibrary::SaveFlowState(
	UObject* WorldContextObject,
	const FString& SaveId,
	const FString& SaveName,
	const FString& FlowName,
	const TArray<FExFlowTaskState>& TaskStates,
	const TMap<FString, FString>& FlowVariables)
{
	if (!WorldContextObject)
	{
		return false;
	}

	USaveGame* SaveGameObject = UGameplayStatics::CreateSaveGameObject(USaveGame::StaticClass());
	if (!SaveGameObject)
	{
		return false;
	}

	FExFlowState FlowState;
	FlowState.SaveId = SaveId;
	FlowState.SaveName = SaveName;
	FlowState.FlowName = FlowName;
	FlowState.CreatedTime = FDateTime::Now();
	FlowState.UpdatedTime = FDateTime::Now();
	FlowState.ActiveTasks = TaskStates;
	FlowState.FlowVariables = FlowVariables;

	FString SlotName = TEXT("FlowSave_") + SaveId;
	return UGameplayStatics::SaveGameToSlot(SaveGameObject, SlotName, 0);
}

FExFlowState UExSaveGameLibrary::LoadFlowState(
	UObject* WorldContextObject,
	const FString& SaveId)
{
	FExFlowState EmptyState;
	
	if (!WorldContextObject)
	{
		return EmptyState;
	}

	FString SlotName = TEXT("FlowSave_") + SaveId;
	USaveGame* LoadGameObject = UGameplayStatics::LoadGameFromSlot(SlotName, 0);
	
	if (!LoadGameObject)
	{
		return EmptyState;
	}

	return EmptyState;
}

TArray<FExFlowState> UExSaveGameLibrary::GetAllFlowSaves(UObject* WorldContextObject)
{
	TArray<FExFlowState> AllSaves;

	if (!WorldContextObject)
	{
		return AllSaves;
	}

	return AllSaves;
}

bool UExSaveGameLibrary::DeleteFlowSave(
	UObject* WorldContextObject,
	const FString& SaveId)
{
	if (!WorldContextObject)
	{
		return false;
	}

	FString SlotName = TEXT("FlowSave_") + SaveId;
	return UGameplayStatics::DeleteGameInSlot(SlotName, 0);
}

bool UExSaveGameLibrary::FlowSaveExists(
	UObject* WorldContextObject,
	const FString& SaveId)
{
	if (!WorldContextObject)
	{
		return false;
	}

	FString SlotName = TEXT("FlowSave_") + SaveId;
	return UGameplayStatics::DoesSaveGameExist(SlotName, 0);
}

FString UExSaveGameLibrary::SerializeTaskState(const FExFlowTaskState& TaskState)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	
	JsonObject->SetStringField(TEXT("TaskId"), TaskState.TaskId);
	JsonObject->SetStringField(TEXT("TaskClassName"), TaskState.TaskClassName);
	JsonObject->SetStringField(TEXT("CurrentState"), TaskState.CurrentState);
	JsonObject->SetStringField(TEXT("TaskParameters"), TaskState.TaskParameters);
	JsonObject->SetNumberField(TEXT("StartTimestamp"), TaskState.StartTimestamp);
	JsonObject->SetNumberField(TEXT("LastUpdateTimestamp"), TaskState.LastUpdateTimestamp);
	JsonObject->SetBoolField(TEXT("bIsCompleted"), TaskState.bIsCompleted);
	JsonObject->SetNumberField(TEXT("CompletedTimestamp"), TaskState.CompletedTimestamp);
	JsonObject->SetNumberField(TEXT("CheckpointIndex"), TaskState.CheckpointIndex);

	FString OutputString;
	TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<TCHAR>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);

	return OutputString;
}

FExFlowTaskState UExSaveGameLibrary::DeserializeTaskState(const FString& JsonString)
{
	FExFlowTaskState TaskState;

	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(JsonString);
	TSharedPtr<FJsonObject> JsonObject;
	
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
	{
		TaskState.TaskId = JsonObject->GetStringField(TEXT("TaskId"));
		TaskState.TaskClassName = JsonObject->GetStringField(TEXT("TaskClassName"));
		TaskState.CurrentState = JsonObject->GetStringField(TEXT("CurrentState"));
		TaskState.TaskParameters = JsonObject->GetStringField(TEXT("TaskParameters"));
		TaskState.StartTimestamp = JsonObject->GetNumberField(TEXT("StartTimestamp"));
		TaskState.LastUpdateTimestamp = JsonObject->GetNumberField(TEXT("LastUpdateTimestamp"));
		TaskState.bIsCompleted = JsonObject->GetBoolField(TEXT("bIsCompleted"));
		TaskState.CompletedTimestamp = JsonObject->GetNumberField(TEXT("CompletedTimestamp"));
		TaskState.CheckpointIndex = static_cast<int32>(JsonObject->GetNumberField(TEXT("CheckpointIndex")));
	}

	return TaskState;
}
