// Copyright Epic Games, Inc. All Rights Reserved.

#include "Quest/ExQuestReplicationComponent.h"

#include "BlueprintTool/Common/ExLatentProxyDefine.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Quest/ExQuestManagerSubsystem.h"

namespace ExQuestReplication
{
	static UExQuestManagerSubsystem* GetManager(const UObject* WorldContextObject)
	{
		if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject))
		{
			return GameInstance->GetSubsystem<UExQuestManagerSubsystem>();
		}
		return nullptr;
	}
}

UExQuestReplicationComponent::UExQuestReplicationComponent()
{
	SetIsReplicatedByDefault(true);
}

UExQuestReplicationComponent* UExQuestReplicationComponent::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	const UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	if (AGameStateBase* GameState = World->GetGameState())
	{
		return GameState->FindComponentByClass<UExQuestReplicationComponent>();
	}

	return nullptr;
}

UExQuestReplicationComponent* UExQuestReplicationComponent::EnsureOnGameState(UObject* WorldContextObject)
{
	if (UExQuestReplicationComponent* Existing = Get(WorldContextObject))
	{
		return Existing;
	}

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return nullptr;
	}

	AGameStateBase* GameState = World->GetGameState();
	if (!GameState)
	{
		UE_LOG(LogBlueprintNodeGraph, Warning,
			TEXT("EnsureOnGameState: GameState not ready yet (call again after GameState exists)"));
		return nullptr;
	}

	if (!GameState->HasAuthority())
	{
		return nullptr;
	}

	UExQuestReplicationComponent* NewComponent = NewObject<UExQuestReplicationComponent>(
		GameState,
		UExQuestReplicationComponent::StaticClass(),
		TEXT("QuestReplication"));
	if (!NewComponent)
	{
		return nullptr;
	}

	NewComponent->SetIsReplicated(true);
	NewComponent->RegisterComponent();

	UE_LOG(LogBlueprintNodeGraph, Log,
		TEXT("EnsureOnGameState: added quest replication to '%s' at runtime"),
		*GetNameSafe(GameState));

	return NewComponent;
}

UExQuestReplicationComponent* UExQuestReplicationComponent::GetForRouting(UObject* WorldContextObject)
{
	if (UExQuestReplicationComponent* Existing = Get(WorldContextObject))
	{
		return Existing;
	}

	return EnsureOnGameState(WorldContextObject);
}

bool UExQuestReplicationComponent::IsAuthorityEndpoint() const
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	if (Owner->GetLocalRole() == ROLE_Authority)
	{
		return true;
	}

	if (const UWorld* World = Owner->GetWorld())
	{
		return World->GetNetMode() == NM_Standalone;
	}

	return false;
}

void UExQuestReplicationComponent::BeginPlay()
{
	Super::BeginPlay();

	if (IsAuthorityEndpoint() && ReplicatedQuestAsset && ReplicatedRuntimeState.QuestSetId.IsEmpty())
	{
		if (UExQuestManagerSubsystem* Manager = ExQuestReplication::GetManager(this))
		{
			PublishStateFromAuthorityManager(Manager);
		}
	}
	else if (!IsAuthorityEndpoint())
	{
		ApplyReplicationToLocalManagers();
	}
}

void UExQuestReplicationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UExQuestReplicationComponent, ReplicatedRuntimeState);
	DOREPLIFETIME(UExQuestReplicationComponent, ReplicatedQuestAsset);
}

void UExQuestReplicationComponent::PublishStateFromAuthorityManager(UExQuestManagerSubsystem* QuestManager)
{
	if (!QuestManager || !IsAuthorityEndpoint())
	{
		return;
	}

	ReplicatedRuntimeState = QuestManager->GetRuntimeState();
	ReplicatedQuestAsset = QuestManager->GetLoadedQuestAsset();
}

void UExQuestReplicationComponent::ApplyReplicationToLocalManagers()
{
	UExQuestManagerSubsystem* Manager = ExQuestReplication::GetManager(this);
	if (!Manager)
	{
		return;
	}

	if (ReplicatedQuestAsset)
	{
		Manager->ApplyReplicatedQuestView(ReplicatedQuestAsset, ReplicatedRuntimeState);
	}
	else if (!ReplicatedRuntimeState.QuestSetId.IsEmpty())
	{
		Manager->ApplyReplicatedQuestView(nullptr, ReplicatedRuntimeState);
	}
}

void UExQuestReplicationComponent::OnRep_ReplicatedRuntimeState()
{
	ApplyReplicationToLocalManagers();
}

void UExQuestReplicationComponent::OnRep_ReplicatedQuestAsset()
{
	ApplyReplicationToLocalManagers();
}

bool UExQuestReplicationComponent::RouteUnlockQuest(UObject* WorldContextObject, const FGameplayTag& TaskId)
{
	if (UExQuestReplicationComponent* Rep = GetForRouting(WorldContextObject))
	{
		if (Rep->IsAuthorityEndpoint())
		{
			if (UExQuestManagerSubsystem* Manager = ExQuestReplication::GetManager(WorldContextObject))
			{
				return Manager->UnlockQuest(TaskId);
			}
			return false;
		}

		Rep->Server_UnlockQuest(TaskId);
		return true;
	}

	if (UExQuestManagerSubsystem* Manager = ExQuestReplication::GetManager(WorldContextObject))
	{
		return Manager->UnlockQuest(TaskId);
	}

	return false;
}

bool UExQuestReplicationComponent::RouteActivateQuest(UObject* WorldContextObject, const FGameplayTag& TaskId)
{
	if (UExQuestReplicationComponent* Rep = GetForRouting(WorldContextObject))
	{
		if (Rep->IsAuthorityEndpoint())
		{
			if (UExQuestManagerSubsystem* Manager = ExQuestReplication::GetManager(WorldContextObject))
			{
				return Manager->ActivateQuest(TaskId);
			}
			return false;
		}

		Rep->Server_ActivateQuest(TaskId);
		return true;
	}

	if (UExQuestManagerSubsystem* Manager = ExQuestReplication::GetManager(WorldContextObject))
	{
		return Manager->ActivateQuest(TaskId);
	}

	return false;
}

bool UExQuestReplicationComponent::RouteCompleteQuest(UObject* WorldContextObject, const FGameplayTag& TaskId)
{
	if (UExQuestReplicationComponent* Rep = GetForRouting(WorldContextObject))
	{
		if (Rep->IsAuthorityEndpoint())
		{
			if (UExQuestManagerSubsystem* Manager = ExQuestReplication::GetManager(WorldContextObject))
			{
				return Manager->CompleteQuest(TaskId);
			}
			return false;
		}

		Rep->Server_CompleteQuest(TaskId);
		return true;
	}

	if (UExQuestManagerSubsystem* Manager = ExQuestReplication::GetManager(WorldContextObject))
	{
		return Manager->CompleteQuest(TaskId);
	}

	return false;
}

bool UExQuestReplicationComponent::RouteIncrementQuestObjective(
	UObject* WorldContextObject,
	const FGameplayTag& TaskId,
	const FGameplayTag& ObjectiveTag,
	int32 Delta)
{
	if (UExQuestReplicationComponent* Rep = GetForRouting(WorldContextObject))
	{
		if (Rep->IsAuthorityEndpoint())
		{
			if (UExQuestManagerSubsystem* Manager = ExQuestReplication::GetManager(WorldContextObject))
			{
				return Manager->IncrementQuestObjective(TaskId, ObjectiveTag, Delta);
			}
			return false;
		}

		Rep->Server_IncrementQuestObjective(TaskId, ObjectiveTag, Delta);
		return true;
	}

	if (UExQuestManagerSubsystem* Manager = ExQuestReplication::GetManager(WorldContextObject))
	{
		return Manager->IncrementQuestObjective(TaskId, ObjectiveTag, Delta);
	}

	return false;
}

bool UExQuestReplicationComponent::RouteCompleteQuestObjective(
	UObject* WorldContextObject,
	const FGameplayTag& TaskId,
	const FGameplayTag& ObjectiveTag)
{
	if (UExQuestReplicationComponent* Rep = GetForRouting(WorldContextObject))
	{
		if (Rep->IsAuthorityEndpoint())
		{
			if (UExQuestManagerSubsystem* Manager = ExQuestReplication::GetManager(WorldContextObject))
			{
				return Manager->CompleteQuestObjective(TaskId, ObjectiveTag);
			}
			return false;
		}

		Rep->Server_CompleteQuestObjective(TaskId, ObjectiveTag);
		return true;
	}

	if (UExQuestManagerSubsystem* Manager = ExQuestReplication::GetManager(WorldContextObject))
	{
		return Manager->CompleteQuestObjective(TaskId, ObjectiveTag);
	}

	return false;
}

bool UExQuestReplicationComponent::RouteNotifyObjectiveProgressByTag(
	UObject* WorldContextObject,
	const FGameplayTag& ObjectiveTag,
	int32 Delta)
{
	if (UExQuestReplicationComponent* Rep = GetForRouting(WorldContextObject))
	{
		if (Rep->IsAuthorityEndpoint())
		{
			if (UExQuestManagerSubsystem* Manager = ExQuestReplication::GetManager(WorldContextObject))
			{
				return Manager->NotifyObjectiveProgressByTag(ObjectiveTag, Delta);
			}
			return false;
		}

		Rep->Server_NotifyObjectiveProgressByTag(ObjectiveTag, Delta);
		return true;
	}

	if (UExQuestManagerSubsystem* Manager = ExQuestReplication::GetManager(WorldContextObject))
	{
		return Manager->NotifyObjectiveProgressByTag(ObjectiveTag, Delta);
	}

	return false;
}

void UExQuestReplicationComponent::RouteApplyRuntimeState(UObject* WorldContextObject, const FExQuestRuntimeState& RuntimeState)
{
	if (UExQuestReplicationComponent* Rep = GetForRouting(WorldContextObject))
	{
		if (Rep->IsAuthorityEndpoint())
		{
			if (UExQuestManagerSubsystem* Manager = ExQuestReplication::GetManager(WorldContextObject))
			{
				Manager->ApplyRuntimeState(RuntimeState);
			}
			return;
		}

		Rep->Server_ApplyRuntimeState(RuntimeState);
		return;
	}

	if (UExQuestManagerSubsystem* Manager = ExQuestReplication::GetManager(WorldContextObject))
	{
		Manager->ApplyRuntimeState(RuntimeState);
	}
}

void UExQuestReplicationComponent::RouteLoadQuestFromAsset(
	UObject* WorldContextObject,
	UExQuestDataAsset* QuestAsset,
	bool bPreserveRuntime)
{
	if (UExQuestReplicationComponent* Rep = GetForRouting(WorldContextObject))
	{
		if (Rep->IsAuthorityEndpoint())
		{
			if (UExQuestManagerSubsystem* Manager = ExQuestReplication::GetManager(WorldContextObject))
			{
				Manager->LoadQuestFromAsset(QuestAsset, bPreserveRuntime);
			}
			return;
		}

		Rep->Server_LoadQuestFromAsset(QuestAsset, bPreserveRuntime);
		return;
	}

	if (UExQuestManagerSubsystem* Manager = ExQuestReplication::GetManager(WorldContextObject))
	{
		Manager->LoadQuestFromAsset(QuestAsset, bPreserveRuntime);
	}
}

bool UExQuestReplicationComponent::RouteLoadQuestProgressFromJson(UObject* WorldContextObject, const FString& JsonSaveData)
{
	if (UExQuestReplicationComponent* Rep = GetForRouting(WorldContextObject))
	{
		if (Rep->IsAuthorityEndpoint())
		{
			if (UExQuestManagerSubsystem* Manager = ExQuestReplication::GetManager(WorldContextObject))
			{
				return Manager->LoadQuestProgressFromJson(JsonSaveData);
			}
			return false;
		}

		Rep->Server_LoadQuestProgressFromJson(JsonSaveData);
		return true;
	}

	if (UExQuestManagerSubsystem* Manager = ExQuestReplication::GetManager(WorldContextObject))
	{
		return Manager->LoadQuestProgressFromJson(JsonSaveData);
	}

	return false;
}

void UExQuestReplicationComponent::RouteApplyObjectiveProgressMessage(
	UObject* WorldContextObject,
	const FExQuestObjectiveProgressMessage& Message)
{
	if (UExQuestReplicationComponent* Rep = GetForRouting(WorldContextObject))
	{
		if (Rep->IsAuthorityEndpoint())
		{
			if (UExQuestManagerSubsystem* Manager = ExQuestReplication::GetManager(WorldContextObject))
			{
				if (Message.TaskId.IsValid())
				{
					Manager->IncrementQuestObjective(Message.TaskId, Message.ObjectiveTag, Message.Delta);
				}
				else
				{
					Manager->NotifyObjectiveProgressByTag(Message.ObjectiveTag, Message.Delta);
				}
			}
			return;
		}

		Rep->Server_ApplyObjectiveProgressMessage(Message);
		return;
	}

	if (UExQuestManagerSubsystem* Manager = ExQuestReplication::GetManager(WorldContextObject))
	{
		if (Message.TaskId.IsValid())
		{
			Manager->IncrementQuestObjective(Message.TaskId, Message.ObjectiveTag, Message.Delta);
		}
		else
		{
			Manager->NotifyObjectiveProgressByTag(Message.ObjectiveTag, Message.Delta);
		}
	}
}

void UExQuestReplicationComponent::Server_UnlockQuest_Implementation(const FGameplayTag& TaskId)
{
	if (UExQuestManagerSubsystem* Manager = ExQuestReplication::GetManager(this))
	{
		Manager->UnlockQuest(TaskId);
	}
}

void UExQuestReplicationComponent::Server_ActivateQuest_Implementation(const FGameplayTag& TaskId)
{
	if (UExQuestManagerSubsystem* Manager = ExQuestReplication::GetManager(this))
	{
		Manager->ActivateQuest(TaskId);
	}
}

void UExQuestReplicationComponent::Server_CompleteQuest_Implementation(const FGameplayTag& TaskId)
{
	if (UExQuestManagerSubsystem* Manager = ExQuestReplication::GetManager(this))
	{
		Manager->CompleteQuest(TaskId);
	}
}

void UExQuestReplicationComponent::Server_IncrementQuestObjective_Implementation(
	const FGameplayTag& TaskId,
	const FGameplayTag& ObjectiveTag,
	int32 Delta)
{
	if (UExQuestManagerSubsystem* Manager = ExQuestReplication::GetManager(this))
	{
		Manager->IncrementQuestObjective(TaskId, ObjectiveTag, Delta);
	}
}

void UExQuestReplicationComponent::Server_CompleteQuestObjective_Implementation(
	const FGameplayTag& TaskId,
	const FGameplayTag& ObjectiveTag)
{
	if (UExQuestManagerSubsystem* Manager = ExQuestReplication::GetManager(this))
	{
		Manager->CompleteQuestObjective(TaskId, ObjectiveTag);
	}
}

void UExQuestReplicationComponent::Server_NotifyObjectiveProgressByTag_Implementation(const FGameplayTag& ObjectiveTag, int32 Delta)
{
	if (UExQuestManagerSubsystem* Manager = ExQuestReplication::GetManager(this))
	{
		Manager->NotifyObjectiveProgressByTag(ObjectiveTag, Delta);
	}
}

void UExQuestReplicationComponent::Server_ApplyRuntimeState_Implementation(const FExQuestRuntimeState& RuntimeState)
{
	if (UExQuestManagerSubsystem* Manager = ExQuestReplication::GetManager(this))
	{
		Manager->ApplyRuntimeState(RuntimeState);
	}
}

void UExQuestReplicationComponent::Server_LoadQuestFromAsset_Implementation(UExQuestDataAsset* QuestAsset, bool bPreserveRuntime)
{
	if (UExQuestManagerSubsystem* Manager = ExQuestReplication::GetManager(this))
	{
		Manager->LoadQuestFromAsset(QuestAsset, bPreserveRuntime);
	}
}

void UExQuestReplicationComponent::Server_LoadQuestProgressFromJson_Implementation(const FString& JsonSaveData)
{
	if (UExQuestManagerSubsystem* Manager = ExQuestReplication::GetManager(this))
	{
		Manager->LoadQuestProgressFromJson(JsonSaveData);
	}
}

void UExQuestReplicationComponent::Server_ApplyObjectiveProgressMessage_Implementation(const FExQuestObjectiveProgressMessage& Message)
{
	if (UExQuestManagerSubsystem* Manager = ExQuestReplication::GetManager(this))
	{
		if (Message.TaskId.IsValid())
		{
			Manager->IncrementQuestObjective(Message.TaskId, Message.ObjectiveTag, Message.Delta);
		}
		else
		{
			Manager->NotifyObjectiveProgressByTag(Message.ObjectiveTag, Message.Delta);
		}
	}
}
