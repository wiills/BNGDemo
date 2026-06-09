#include "ScopedMessageSubsystem.h"

#include "Components/ActorComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "Misc/Guid.h"
#include "ScopeContextProvider.h"
#include "ScopedMessageClientBridgeComponent.h"
#include "ScopedMessageReplicatorComponent.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "UObject/StructOnScope.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY(LogScopedMessageSubsystem);

UScopedMessageSubsystem& UScopedMessageSubsystem::Get(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	UGameInstance* GameInstance = World->GetGameInstance();
	check(GameInstance);
	return *GameInstance->GetSubsystem<UScopedMessageSubsystem>();
}

bool UScopedMessageSubsystem::HasInstance(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance && GameInstance->GetSubsystem<UScopedMessageSubsystem>() != nullptr;
}

FString UScopedMessageSubsystem::GetNetModePrefix(const UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull))
	{
		switch (World->GetNetMode())
		{
		case NM_Client:
			return TEXT("Client");
		case NM_DedicatedServer:
		case NM_ListenServer:
			return TEXT("Server");
		case NM_Standalone:
			return TEXT("Standalone");
		default:
			break;
		}
	}
	return TEXT("Unknown");
}

void UScopedMessageSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UScopedMessageSubsystem::OnWorldInitialized);
}

void UScopedMessageSubsystem::Deinitialize()
{
	FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);
	RoutingTable.Empty();
	LocalScopeMap.Empty();
	ScopePlayerInterests.Empty();
	Super::Deinitialize();
}

static FScopedMessageScopeId FindScopeIdOnObject(UObject* Object)
{
	if (!Object)
	{
		return FScopedMessageScopeId();
	}

	if (Object->GetClass()->ImplementsInterface(UScopeContextProvider::StaticClass()))
	{
		const FScopedMessageScopeId ScopeId = IScopeContextProvider::Execute_GetScopeId(Object);
		if (ScopeId.IsValid())
		{
			return ScopeId;
		}
	}

	if (UActorComponent* Component = Cast<UActorComponent>(Object))
	{
		if (AActor* Owner = Component->GetOwner())
		{
			return FindScopeIdOnObject(Owner);
		}
	}

	if (AActor* Actor = Cast<AActor>(Object))
	{
		TInlineComponentArray<UActorComponent*> Components;
		Actor->GetComponents(Components);
		for (UActorComponent* Component : Components)
		{
			if (Component && Component->GetClass()->ImplementsInterface(UScopeContextProvider::StaticClass()))
			{
				const FScopedMessageScopeId ScopeId = IScopeContextProvider::Execute_GetScopeId(Component);
				if (ScopeId.IsValid())
				{
					return ScopeId;
				}
			}
		}
	}

	return FScopedMessageScopeId();
}

FScopedMessageScopeId UScopedMessageSubsystem::GetOrCreateLocalScopeId(const UObject* ScopeObject)
{
	if (!ScopeObject)
	{
		return FScopedMessageScopeId();
	}

	UObject* NonConstScopeObject = const_cast<UObject*>(ScopeObject);
	TWeakObjectPtr<UObject> WeakKey(NonConstScopeObject);
	if (const FScopedMessageScopeId* FoundScopeId = LocalScopeMap.Find(WeakKey))
	{
		if (FoundScopeId->IsValid())
		{
			return *FoundScopeId;
		}
	}

	const FString GeneratedName = FString::Printf(TEXT("Local.%s"), *FGuid::NewGuid().ToString(EGuidFormats::Digits));
	const FScopedMessageScopeId NewScopeId{FName(*GeneratedName)};
	LocalScopeMap.Add(WeakKey, NewScopeId);
	return NewScopeId;
}

FScopedMessageScopeId UScopedMessageSubsystem::ResolveScopeId(UObject* ScopeContext) const
{
	if (!ScopeContext)
	{
		return FScopedMessageScopeId();
	}

	if (const FScopedMessageScopeId DirectScopeId = FindScopeIdOnObject(ScopeContext); DirectScopeId.IsValid())
	{
		return DirectScopeId;
	}

	if (AActor* Actor = Cast<AActor>(ScopeContext))
	{
		AActor* CurrentOwner = Actor->GetOwner();
		while (CurrentOwner)
		{
			if (const FScopedMessageScopeId ScopeId = FindScopeIdOnObject(CurrentOwner); ScopeId.IsValid())
			{
				return ScopeId;
			}
			CurrentOwner = CurrentOwner->GetOwner();
		}

		AActor* Parent = Actor->GetAttachParentActor();
		while (Parent)
		{
			if (const FScopedMessageScopeId ScopeId = FindScopeIdOnObject(Parent); ScopeId.IsValid())
			{
				return ScopeId;
			}
			Parent = Parent->GetAttachParentActor();
		}
	}

	UObject* Current = ScopeContext->GetOuter();
	while (Current)
	{
		if (const FScopedMessageScopeId ScopeId = FindScopeIdOnObject(Current); ScopeId.IsValid())
		{
			return ScopeId;
		}
		Current = Current->GetOuter();
	}

	return FScopedMessageScopeId();
}

bool UScopedMessageSubsystem::IsAuthorityOrStandalone() const
{
	const UWorld* World = GetWorld();
	return World && World->GetNetMode() != NM_Client;
}

bool UScopedMessageSubsystem::IsClientNetMode() const
{
	const UWorld* World = GetWorld();
	return World && World->GetNetMode() == NM_Client;
}

void UScopedMessageSubsystem::BroadcastMessageInternal(
	FGameplayTag Channel,
	const UScriptStruct* PayloadType,
	const void* PayloadBytes,
	FScopedMessageScopeId ScopeId,
	EScopedMessageReplication Replication)
{
	if (!Channel.IsValid())
	{
		UE_LOG(LogScopedMessageSubsystem, Warning, TEXT("[%s] Broadcast called with invalid Channel"), *GetNetModePrefix(this));
		return;
	}

	if (!PayloadType || !PayloadBytes)
	{
		UE_LOG(LogScopedMessageSubsystem, Warning, TEXT("[%s] Broadcast called with null PayloadType or PayloadBytes"), *GetNetModePrefix(this));
		return;
	}

	if (!ScopeId.IsValid())
	{
		UE_LOG(LogScopedMessageSubsystem, Warning, TEXT("[%s] Broadcasting channel %s with empty ScopeId; this behaves as a global scope"),
			*GetNetModePrefix(this), *Channel.ToString());
	}

	switch (Replication)
	{
	case EScopedMessageReplication::LocalOnly:
		DispatchMessage(ScopeId, Channel, PayloadType, PayloadBytes);
		return;

	case EScopedMessageReplication::ServerOnly:
		if (IsAuthorityOrStandalone())
		{
			DispatchMessage(ScopeId, Channel, PayloadType, PayloadBytes);
		}
		else
		{
			UE_LOG(LogScopedMessageSubsystem, Warning, TEXT("[%s] ServerOnly message %s ignored on client"), *GetNetModePrefix(this), *Channel.ToString());
		}
		return;

	case EScopedMessageReplication::ClientToServer:
		if (IsClientNetMode())
		{
			FScopedMessageNetworkPacket Packet;
			if (BuildNetworkPacket(ScopeId, Channel, PayloadType, PayloadBytes, Packet))
			{
				SendPacketToServer(Packet);
			}
		}
		else
		{
			DispatchMessage(ScopeId, Channel, PayloadType, PayloadBytes);
		}
		return;

	case EScopedMessageReplication::ServerToAllClients:
	case EScopedMessageReplication::ServerToScopedClients:
		if (!IsAuthorityOrStandalone())
		{
			UE_LOG(LogScopedMessageSubsystem, Warning, TEXT("[%s] Server-to-client message %s must be sent from authority"),
				*GetNetModePrefix(this), *Channel.ToString());
			return;
		}

		DispatchMessage(ScopeId, Channel, PayloadType, PayloadBytes);

		if (GetWorld() && GetWorld()->GetNetMode() != NM_Standalone)
		{
			FScopedMessageNetworkPacket Packet;
			if (BuildNetworkPacket(ScopeId, Channel, PayloadType, PayloadBytes, Packet))
			{
				if (Replication == EScopedMessageReplication::ServerToAllClients)
				{
					SendPacketToAllClients(Packet);
				}
				else
				{
					SendPacketToScopedClients(Packet);
				}
			}
		}
		return;
	}
}

void UScopedMessageSubsystem::DispatchMessage(
	FScopedMessageScopeId ScopeId,
	FGameplayTag Channel,
	const UScriptStruct* PayloadType,
	const void* PayloadBytes)
{
	FScopeChannelMap* ScopeMap = RoutingTable.Find(ScopeId);
	if (!ScopeMap)
	{
		return;
	}

	TArray<FGameplayTag> ChannelsToCleanup;
	for (TPair<FGameplayTag, FListenerList>& ChannelPair : ScopeMap->ChannelMap)
	{
		const FGameplayTag RegisteredChannel = ChannelPair.Key;
		FListenerList& ListenerList = ChannelPair.Value;
		const bool bExactChannel = RegisteredChannel == Channel;
		const bool bPartialChannel = Channel.MatchesTag(RegisteredChannel);

		if (!bExactChannel && !bPartialChannel)
		{
			continue;
		}

		TArray<FScopedMessageListenerData> ListenersCopy = ListenerList.Listeners;
		for (FScopedMessageListenerData& Listener : ListenersCopy)
		{
			if (!Listener.Owner.IsValid())
			{
				continue;
			}

			const bool bListenerMatches =
				bExactChannel ||
				(Listener.MatchType == EScopedMessageMatch::PartialMatch && bPartialChannel);
			if (!bListenerMatches)
			{
				continue;
			}

			if (Listener.PayloadType.IsValid() && Listener.PayloadType.Get() != PayloadType)
			{
				UE_LOG(LogScopedMessageSubsystem, Warning,
					TEXT("[%s] Payload type mismatch for channel %s: expected %s, got %s"),
					*GetNetModePrefix(this),
					*Channel.ToString(),
					*GetNameSafe(Listener.PayloadType.Get()),
					*GetNameSafe(PayloadType));
				continue;
			}

			Listener.Callback(Channel, PayloadType, PayloadBytes);
		}

		ChannelsToCleanup.Add(RegisteredChannel);
	}

	for (const FGameplayTag CleanupChannel : ChannelsToCleanup)
	{
		CleanupInvalidListeners(ScopeId, CleanupChannel);
	}
}

FScopedMessageListenerHandle UScopedMessageSubsystem::SubscribeInternal(
	FGameplayTag Channel,
	TFunction<void(FGameplayTag, const UScriptStruct*, const void*)>&& Callback,
	const UScriptStruct* PayloadType,
	FScopedMessageScopeId ScopeId,
	EScopedMessageMatch MatchType,
	UObject* Owner)
{
	if (!Channel.IsValid())
	{
		UE_LOG(LogScopedMessageSubsystem, Warning, TEXT("Subscribe called with invalid Channel"));
		return FScopedMessageListenerHandle();
	}

	FScopeChannelMap& ScopeMap = RoutingTable.FindOrAdd(ScopeId);
	FListenerList& ListenerList = ScopeMap.ChannelMap.FindOrAdd(Channel);

	const int32 NewHandleID = ListenerList.NextHandleID++;
	FScopedMessageListenerData& NewListener = ListenerList.Listeners.AddDefaulted_GetRef();
	NewListener.Callback = MoveTemp(Callback);
	NewListener.HandleID = NewHandleID;
	NewListener.MatchType = MatchType;
	NewListener.Owner = Owner;
	NewListener.PayloadType = PayloadType;

	return FScopedMessageListenerHandle(this, ScopeId, Channel, NewHandleID);
}

void UScopedMessageSubsystem::Unsubscribe(FScopedMessageListenerHandle& Handle)
{
	if (Handle.IsValid())
	{
		UnsubscribeInternal(Handle.ScopeId, Handle.Channel, Handle.HandleID);
		Handle = FScopedMessageListenerHandle();
	}
}

void UScopedMessageSubsystem::UnsubscribeInternal(FScopedMessageScopeId ScopeId, FGameplayTag Channel, int32 HandleID)
{
	FScopeChannelMap* ScopeMap = RoutingTable.Find(ScopeId);
	if (!ScopeMap)
	{
		return;
	}

	FListenerList* ListenerList = ScopeMap->ChannelMap.Find(Channel);
	if (!ListenerList)
	{
		return;
	}

	ListenerList->Listeners.RemoveAll([HandleID](const FScopedMessageListenerData& Listener)
	{
		return Listener.HandleID == HandleID;
	});

	if (ListenerList->Listeners.Num() == 0)
	{
		ScopeMap->ChannelMap.Remove(Channel);
		if (ScopeMap->ChannelMap.Num() == 0)
		{
			RoutingTable.Remove(ScopeId);
		}
	}
}

void UScopedMessageSubsystem::CleanupInvalidListeners(FScopedMessageScopeId ScopeId, FGameplayTag Channel)
{
	for (auto It = LocalScopeMap.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid())
		{
			It.RemoveCurrent();
		}
	}

	FScopeChannelMap* ScopeMap = RoutingTable.Find(ScopeId);
	if (!ScopeMap)
	{
		return;
	}

	FListenerList* ListenerList = ScopeMap->ChannelMap.Find(Channel);
	if (!ListenerList)
	{
		return;
	}

	ListenerList->Listeners.RemoveAll([](const FScopedMessageListenerData& Listener)
	{
		return !Listener.Owner.IsValid();
	});

	if (ListenerList->Listeners.Num() == 0)
	{
		ScopeMap->ChannelMap.Remove(Channel);
		if (ScopeMap->ChannelMap.Num() == 0)
		{
			RoutingTable.Remove(ScopeId);
		}
	}
}

bool UScopedMessageSubsystem::BuildNetworkPacket(
	FScopedMessageScopeId ScopeId,
	FGameplayTag Channel,
	const UScriptStruct* PayloadType,
	const void* PayloadBytes,
	FScopedMessageNetworkPacket& OutPacket) const
{
	if (!PayloadType || !PayloadBytes)
	{
		return false;
	}

	OutPacket.ScopeId = ScopeId;
	OutPacket.Channel = Channel;
	OutPacket.Payload.PayloadStructPath = PayloadType->GetPathName();
	OutPacket.Payload.PayloadBytes.Reset();

	FMemoryWriter Writer(OutPacket.Payload.PayloadBytes);
	const_cast<UScriptStruct*>(PayloadType)->SerializeItem(Writer, const_cast<void*>(PayloadBytes), nullptr);
	return true;
}

const UScriptStruct* UScopedMessageSubsystem::ResolvePayloadType(const FScopedMessageNetworkPacket& Packet) const
{
	if (Packet.Payload.PayloadStructPath.IsEmpty())
	{
		return nullptr;
	}

	if (UScriptStruct* Struct = FindObject<UScriptStruct>(nullptr, *Packet.Payload.PayloadStructPath))
	{
		return Struct;
	}

	return LoadObject<UScriptStruct>(nullptr, *Packet.Payload.PayloadStructPath);
}

bool UScopedMessageSubsystem::DeserializePacket(const FScopedMessageNetworkPacket& Packet, FStructOnScope& OutMessage) const
{
	const UScriptStruct* PayloadType = ResolvePayloadType(Packet);
	if (!PayloadType)
	{
		UE_LOG(LogScopedMessageSubsystem, Warning, TEXT("[%s] Could not resolve payload type %s for channel %s"),
			*GetNetModePrefix(this), *Packet.Payload.PayloadStructPath, *Packet.Channel.ToString());
		return false;
	}

	OutMessage.Initialize(PayloadType);
	FMemoryReader Reader(Packet.Payload.PayloadBytes);
	const_cast<UScriptStruct*>(PayloadType)->SerializeItem(Reader, OutMessage.GetStructMemory(), nullptr);
	return true;
}

void UScopedMessageSubsystem::HandleNetworkMessage(const FScopedMessageNetworkPacket& Packet)
{
	FStructOnScope Message;
	if (!DeserializePacket(Packet, Message))
	{
		return;
	}

	const UScriptStruct* PayloadType = Cast<UScriptStruct>(const_cast<UStruct*>(Message.GetStruct()));
	DispatchMessage(Packet.ScopeId, Packet.Channel, PayloadType, Message.GetStructMemory());
}

void UScopedMessageSubsystem::HandleClientMessage(APlayerController* Sender, const FScopedMessageNetworkPacket& Packet)
{
	if (!Sender)
	{
		return;
	}

	if (!Packet.ScopeId.IsValid() || !IsPlayerRegisteredForScope(Sender, Packet.ScopeId))
	{
		UE_LOG(LogScopedMessageSubsystem, Warning, TEXT("[%s] Rejected client message %s for unregistered scope %s from %s"),
			*GetNetModePrefix(this),
			*Packet.Channel.ToString(),
			*Packet.ScopeId.ToString(),
			*GetNameSafe(Sender));
		return;
	}

	HandleNetworkMessage(Packet);
}

void UScopedMessageSubsystem::K2_BroadcastMessage(
	const UObject* WorldContextObject,
	FGameplayTag Channel,
	const FScopedMessagePayload& Payload,
	EScopedMessageReplication Replication)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogScopedMessageSubsystem, Warning, TEXT("[Unknown] K2_BroadcastMessage called with null WorldContextObject"));
		return;
	}

	if (!Payload.IsValid())
	{
		UE_LOG(LogScopedMessageSubsystem, Warning, TEXT("[%s] K2_BroadcastMessage called with invalid Payload"), *GetNetModePrefix(WorldContextObject));
		return;
	}

	if (HasInstance(WorldContextObject))
	{
		UScopedMessageSubsystem& Subsystem = Get(WorldContextObject);
		FScopedMessageNetworkPacket Packet;
		Packet.ScopeId = Subsystem.ResolveScopeId(const_cast<UObject*>(WorldContextObject));
		Packet.Channel = Channel;
		Packet.Payload = Payload;

		FStructOnScope Message;
		if (!Subsystem.DeserializePacket(Packet, Message))
		{
			return;
		}

		const UScriptStruct* PayloadType = Cast<UScriptStruct>(const_cast<UStruct*>(Message.GetStruct()));
		Subsystem.BroadcastMessageInternal(
			Channel,
			PayloadType,
			Message.GetStructMemory(),
			Packet.ScopeId,
			Replication);
	}
}

void UScopedMessageSubsystem::K2_RegisterPlayerForScope(
	const UObject* WorldContextObject,
	APlayerController* PlayerController,
	FScopedMessageScopeId ScopeId)
{
	if (HasInstance(WorldContextObject))
	{
		Get(WorldContextObject).RegisterPlayerForScope(PlayerController, ScopeId);
	}
}

void UScopedMessageSubsystem::K2_UnregisterPlayerForScope(
	const UObject* WorldContextObject,
	APlayerController* PlayerController,
	FScopedMessageScopeId ScopeId)
{
	if (HasInstance(WorldContextObject))
	{
		Get(WorldContextObject).UnregisterPlayerForScope(PlayerController, ScopeId);
	}
}

void UScopedMessageSubsystem::RegisterPlayerForScope(APlayerController* PlayerController, FScopedMessageScopeId ScopeId)
{
	if (!PlayerController || !ScopeId.IsValid())
	{
		return;
	}

	CreateClientBridgeOnPlayerController(PlayerController);

	TArray<TWeakObjectPtr<APlayerController>>& Players = ScopePlayerInterests.FindOrAdd(ScopeId);
	const bool bAlreadyRegistered = Players.ContainsByPredicate([PlayerController](const TWeakObjectPtr<APlayerController>& ExistingPlayer)
	{
		return ExistingPlayer.Get() == PlayerController;
	});

	if (!bAlreadyRegistered)
	{
		Players.Add(PlayerController);
	}
}

void UScopedMessageSubsystem::UnregisterPlayerForScope(APlayerController* PlayerController, FScopedMessageScopeId ScopeId)
{
	if (!PlayerController || !ScopeId.IsValid())
	{
		return;
	}

	if (TArray<TWeakObjectPtr<APlayerController>>* Players = ScopePlayerInterests.Find(ScopeId))
	{
		Players->RemoveAll([PlayerController](const TWeakObjectPtr<APlayerController>& ExistingPlayer)
		{
			return !ExistingPlayer.IsValid() || ExistingPlayer.Get() == PlayerController;
		});

		if (Players->Num() == 0)
		{
			ScopePlayerInterests.Remove(ScopeId);
		}
	}
}

bool UScopedMessageSubsystem::IsPlayerRegisteredForScope(APlayerController* PlayerController, FScopedMessageScopeId ScopeId) const
{
	const TArray<TWeakObjectPtr<APlayerController>>* Players = ScopePlayerInterests.Find(ScopeId);
	if (!Players)
	{
		return false;
	}

	return Players->ContainsByPredicate([PlayerController](const TWeakObjectPtr<APlayerController>& ExistingPlayer)
	{
		return ExistingPlayer.Get() == PlayerController;
	});
}

void UScopedMessageSubsystem::CleanupInvalidPlayerInterests()
{
	for (auto It = ScopePlayerInterests.CreateIterator(); It; ++It)
	{
		It.Value().RemoveAll([](const TWeakObjectPtr<APlayerController>& Player)
		{
			return !Player.IsValid();
		});

		if (It.Value().Num() == 0)
		{
			It.RemoveCurrent();
		}
	}
}

void UScopedMessageSubsystem::SendPacketToServer(const FScopedMessageNetworkPacket& Packet)
{
	UWorld* World = GetWorld();
	APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
	if (UScopedMessageClientBridgeComponent* Bridge = FindClientBridgeForPlayer(PlayerController))
	{
		Bridge->Server_SendScopedMessage(Packet);
	}
	else
	{
		UE_LOG(LogScopedMessageSubsystem, Warning, TEXT("[%s] No scoped message client bridge found for local player"), *GetNetModePrefix(this));
	}
}

void UScopedMessageSubsystem::SendPacketToAllClients(const FScopedMessageNetworkPacket& Packet)
{
	if (UScopedMessageReplicatorComponent* Replicator = FindReplicator())
	{
		Replicator->NetMulticast_BroadcastMessage(Packet);
	}
	else
	{
		UE_LOG(LogScopedMessageSubsystem, Warning, TEXT("[%s] GameState scoped message replicator not found"), *GetNetModePrefix(this));
	}
}

void UScopedMessageSubsystem::SendPacketToScopedClients(const FScopedMessageNetworkPacket& Packet)
{
	CleanupInvalidPlayerInterests();

	const TArray<TWeakObjectPtr<APlayerController>>* Players = ScopePlayerInterests.Find(Packet.ScopeId);
	if (!Players || Players->Num() == 0)
	{
		UE_LOG(LogScopedMessageSubsystem, Warning, TEXT("[%s] No registered players for scoped message %s Scope=%s"),
			*GetNetModePrefix(this), *Packet.Channel.ToString(), *Packet.ScopeId.ToString());
		return;
	}

	for (const TWeakObjectPtr<APlayerController>& PlayerPtr : *Players)
	{
		if (APlayerController* PlayerController = PlayerPtr.Get())
		{
			if (UScopedMessageClientBridgeComponent* Bridge = FindClientBridgeForPlayer(PlayerController))
			{
				Bridge->Client_ReceiveScopedMessage(Packet);
			}
		}
	}
}

UScopedMessageClientBridgeComponent* UScopedMessageSubsystem::FindClientBridgeForPlayer(APlayerController* PlayerController) const
{
	return PlayerController ? PlayerController->FindComponentByClass<UScopedMessageClientBridgeComponent>() : nullptr;
}

UScopedMessageReplicatorComponent* UScopedMessageSubsystem::FindReplicator() const
{
	UWorld* World = GetWorld();
	AGameStateBase* GameState = World ? World->GetGameState() : nullptr;
	return GameState ? GameState->FindComponentByClass<UScopedMessageReplicatorComponent>() : nullptr;
}

void UScopedMessageSubsystem::OnWorldInitialized(UWorld* World, const UWorld::InitializationValues IValues)
{
	if (!World)
	{
		return;
	}

	World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateUObject(this, &UScopedMessageSubsystem::OnActorSpawned));

	if (AGameStateBase* GameState = World->GetGameState())
	{
		CreateReplicatorOnGameState(GameState);
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PlayerController = It->Get())
		{
			CreateClientBridgeOnPlayerController(PlayerController);
		}
	}
}

void UScopedMessageSubsystem::OnActorSpawned(AActor* SpawnedActor)
{
	if (AGameStateBase* GameState = Cast<AGameStateBase>(SpawnedActor))
	{
		CreateReplicatorOnGameState(GameState);
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(SpawnedActor))
	{
		CreateClientBridgeOnPlayerController(PlayerController);
	}
}

void UScopedMessageSubsystem::CreateReplicatorOnGameState(AGameStateBase* GameState)
{
	if (GameState && GameState->HasAuthority() && !GameState->FindComponentByClass<UScopedMessageReplicatorComponent>())
	{
		UScopedMessageReplicatorComponent* Comp = NewObject<UScopedMessageReplicatorComponent>(GameState, TEXT("ScopedMessageReplicator"));
		Comp->SetIsReplicated(true);
		GameState->AddInstanceComponent(Comp);
		Comp->RegisterComponent();
	}
}

void UScopedMessageSubsystem::CreateClientBridgeOnPlayerController(APlayerController* PlayerController)
{
	if (PlayerController && PlayerController->HasAuthority() && !PlayerController->FindComponentByClass<UScopedMessageClientBridgeComponent>())
	{
		UScopedMessageClientBridgeComponent* Comp = NewObject<UScopedMessageClientBridgeComponent>(PlayerController, TEXT("ScopedMessageClientBridge"));
		Comp->SetIsReplicated(true);
		PlayerController->AddInstanceComponent(Comp);
		Comp->RegisterComponent();
	}
}
