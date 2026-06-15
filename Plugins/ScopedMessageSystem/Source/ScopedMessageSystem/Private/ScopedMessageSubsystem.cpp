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

UScopedMessageSubsystem* UScopedMessageSubsystem::GetInstance(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UScopedMessageSubsystem>() : nullptr;
}

bool UScopedMessageSubsystem::HasInstance(const UObject* WorldContextObject)
{
	return GetInstance(WorldContextObject) != nullptr;
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
	FWorldDelegates::OnWorldCleanup.AddUObject(this, &UScopedMessageSubsystem::OnWorldCleanup);
}

void UScopedMessageSubsystem::Deinitialize()
{
	FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);
	FWorldDelegates::OnWorldCleanup.RemoveAll(this);

	for (const TPair<TWeakObjectPtr<UWorld>, FDelegateHandle>& Entry : ActorSpawnedDelegateHandles)
	{
		if (UWorld* World = Entry.Key.Get())
		{
			World->RemoveOnActorSpawnedHandler(Entry.Value);
		}
	}
	ActorSpawnedDelegateHandles.Empty();
	EndPlayBoundActors.Empty();

	RoutingTable.Empty();
	LocalScopeMap.Empty();
	ScopePlayerInterests.Empty();
	CustomScopeResolvers.Empty();
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
		// Direct providers win over structural traversal. This lets project actors
		// expose a ScopeId without requiring a specific component layout.
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
		// Components are checked here because the common Poi root setup exposes its
		// scope through UScopedMessageScopeComponent rather than the actor itself.
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
	// Custom resolvers are the project escape hatch. They are intentionally tried
	// before the built-in traversal so games can map arbitrary gameplay objects to
	// a scope without changing their ownership or attachment model.
	for (const FRegisteredScopeResolver& RegisteredResolver : CustomScopeResolvers)
	{
		if (!RegisteredResolver.Resolver.IsBound())
		{
			continue;
		}

		FScopedMessageScopeId ResolvedScopeId;
		if (RegisteredResolver.Resolver.Execute(ScopeContext, ResolvedScopeId) && ResolvedScopeId.IsValid())
		{
			return ResolvedScopeId;
		}
	}

	return ResolveScopeIdDefault(ScopeContext);
}

FScopedMessageScopeId UScopedMessageSubsystem::ResolveScopeIdDefault(UObject* ScopeContext) const
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
		// Owner and attachment chains cover the usual "actor belongs to a Poi root"
		// layouts while still keeping the message API independent from any Poi class.
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

	// UObject outers are checked last so non-actor helper objects created under a
	// scoped actor/component can still participate in scoped messaging.
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

FDelegateHandle UScopedMessageSubsystem::RegisterScopeResolver(FScopedMessageScopeResolver Resolver)
{
	if (!Resolver.IsBound())
	{
		return FDelegateHandle();
	}

	FRegisteredScopeResolver& RegisteredResolver = CustomScopeResolvers.AddDefaulted_GetRef();
	RegisteredResolver.Handle = FDelegateHandle(FDelegateHandle::GenerateNewHandle);
	RegisteredResolver.Resolver = MoveTemp(Resolver);
	return RegisteredResolver.Handle;
}

void UScopedMessageSubsystem::UnregisterScopeResolver(FDelegateHandle ResolverHandle)
{
	CustomScopeResolvers.RemoveAll([ResolverHandle](const FRegisteredScopeResolver& RegisteredResolver)
	{
		return RegisteredResolver.Handle == ResolverHandle;
	});
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
			// Client-originated packets are validated on the server against
			// ScopePlayerInterests before being dispatched.
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
			// Server broadcasts always execute locally first. Network replication is
			// an additional delivery path for remote clients.
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

		// Copy the listener array before invoking callbacks so listeners can safely
		// unregister or add other listeners while a message is being dispatched.
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

	if (AActor* OwnerActor = Cast<AActor>(Owner))
	{
		// Actor listeners are removed eagerly on EndPlay. Non-actor UObject owners
		// are held weakly and cleaned during normal sweeps.
		TWeakObjectPtr<AActor> WeakOwnerActor(OwnerActor);
		if (!EndPlayBoundActors.Contains(WeakOwnerActor))
		{
			OwnerActor->OnEndPlay.AddDynamic(this, &UScopedMessageSubsystem::OnActorEndPlay);
			EndPlayBoundActors.Add(WeakOwnerActor);
		}
	}

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

void UScopedMessageSubsystem::CleanupInvalidListenersForOwner(const UObject* Owner)
{
	if (!Owner)
	{
		return;
	}

	for (auto ScopeIt = RoutingTable.CreateIterator(); ScopeIt; ++ScopeIt)
	{
		FScopeChannelMap& ScopeMap = ScopeIt.Value();
		for (auto ChannelIt = ScopeMap.ChannelMap.CreateIterator(); ChannelIt; ++ChannelIt)
		{
			FListenerList& ListenerList = ChannelIt.Value();
			ListenerList.Listeners.RemoveAll([Owner](const FScopedMessageListenerData& Listener)
			{
				return !Listener.Owner.IsValid() || Listener.Owner.Get() == Owner;
			});

			if (ListenerList.Listeners.Num() == 0)
			{
				ChannelIt.RemoveCurrent();
			}
		}

		if (ScopeMap.ChannelMap.Num() == 0)
		{
			ScopeIt.RemoveCurrent();
		}
	}
}

void UScopedMessageSubsystem::CleanupAllInvalidListeners()
{
	for (auto ScopeIt = RoutingTable.CreateIterator(); ScopeIt; ++ScopeIt)
	{
		FScopeChannelMap& ScopeMap = ScopeIt.Value();
		for (auto ChannelIt = ScopeMap.ChannelMap.CreateIterator(); ChannelIt; ++ChannelIt)
		{
			FListenerList& ListenerList = ChannelIt.Value();
			ListenerList.Listeners.RemoveAll([](const FScopedMessageListenerData& Listener)
			{
				return !Listener.Owner.IsValid();
			});

			if (ListenerList.Listeners.Num() == 0)
			{
				ChannelIt.RemoveCurrent();
			}
		}

		if (ScopeMap.ChannelMap.Num() == 0)
		{
			ScopeIt.RemoveCurrent();
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

	// Serialize through reflection so payload structs do not need a custom network
	// serializer at the plugin layer.
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
	// FStructOnScope owns temporary storage for the deserialized payload during
	// this dispatch. Blueprint listeners receive a copied FInstancedStruct later.
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
		// ClientToServer is intentionally gated by interest registration. Without
		// this, any client could inject messages into any known ScopeId.
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
	const FInstancedStruct& Payload,
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
		Subsystem.BroadcastMessageInternal(
			Channel,
			Payload.GetScriptStruct(),
			Payload.GetMemory(),
			Subsystem.ResolveScopeId(const_cast<UObject*>(WorldContextObject)),
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

void UScopedMessageSubsystem::DumpRoutingTable() const
{
	UE_LOG(LogScopedMessageSubsystem, Log, TEXT("[%s] ScopedMessage routing dump: Scopes=%d CustomResolvers=%d"),
		*GetNetModePrefix(this), RoutingTable.Num(), CustomScopeResolvers.Num());

	for (const TPair<FScopedMessageScopeId, FScopeChannelMap>& ScopePair : RoutingTable)
	{
		int32 ListenerCount = 0;
		for (const TPair<FGameplayTag, FListenerList>& ChannelPair : ScopePair.Value.ChannelMap)
		{
			ListenerCount += ChannelPair.Value.Listeners.Num();
		}

		UE_LOG(LogScopedMessageSubsystem, Log, TEXT("  Scope=%s Channels=%d Listeners=%d"),
			*ScopePair.Key.ToString(), ScopePair.Value.ChannelMap.Num(), ListenerCount);

		for (const TPair<FGameplayTag, FListenerList>& ChannelPair : ScopePair.Value.ChannelMap)
		{
			UE_LOG(LogScopedMessageSubsystem, Log, TEXT("    Channel=%s Listeners=%d"),
				*ChannelPair.Key.ToString(), ChannelPair.Value.Listeners.Num());
		}
	}
}

void UScopedMessageSubsystem::DumpScopeResolution(UObject* ScopeContext) const
{
	UE_LOG(LogScopedMessageSubsystem, Log, TEXT("[%s] ScopedMessage scope resolution for %s"),
		*GetNetModePrefix(this), *GetNameSafe(ScopeContext));

	for (const FRegisteredScopeResolver& RegisteredResolver : CustomScopeResolvers)
	{
		FScopedMessageScopeId ResolvedScopeId;
		const bool bResolved = RegisteredResolver.Resolver.IsBound() && RegisteredResolver.Resolver.Execute(ScopeContext, ResolvedScopeId);
		UE_LOG(LogScopedMessageSubsystem, Log, TEXT("  CustomResolver ValidHandle=%s Resolved=%s Scope=%s"),
			RegisteredResolver.Handle.IsValid() ? TEXT("true") : TEXT("false"),
			bResolved ? TEXT("true") : TEXT("false"),
			*ResolvedScopeId.ToString());
	}

	const FScopedMessageScopeId DefaultScopeId = ResolveScopeIdDefault(ScopeContext);
	UE_LOG(LogScopedMessageSubsystem, Log, TEXT("  DefaultResolver Scope=%s"), *DefaultScopeId.ToString());
}

void UScopedMessageSubsystem::RegisterPlayerForScope(APlayerController* PlayerController, FScopedMessageScopeId ScopeId)
{
	if (!PlayerController || !ScopeId.IsValid())
	{
		return;
	}

	CreateClientBridgeOnPlayerController(PlayerController);

	// Interest sets are weak and idempotent; registering the same player twice is
	// harmless and useful for simple enter/stream-in flows.
	TArray<TWeakObjectPtr<APlayerController>>& Players = ScopePlayerInterests.FindOrAdd(ScopeId);
	const bool bAlreadyRegistered = Players.ContainsByPredicate([PlayerController](const TWeakObjectPtr<APlayerController>& ExistingPlayer)
	{
		return ExistingPlayer.Get() == PlayerController;
	});

	if (!bAlreadyRegistered)
	{
		Players.Add(PlayerController);
		NotifyScopeOccupancyChanged(ScopeId);
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
		const int32 NumRemoved = Players->RemoveAll([PlayerController](const TWeakObjectPtr<APlayerController>& ExistingPlayer)
		{
			return !ExistingPlayer.IsValid() || ExistingPlayer.Get() == PlayerController;
		});

		if (Players->Num() == 0)
		{
			ScopePlayerInterests.Remove(ScopeId);
		}

		if (NumRemoved > 0)
		{
			NotifyScopeOccupancyChanged(ScopeId);
		}
	}
}

int32 UScopedMessageSubsystem::GetScopePlayerCount(FScopedMessageScopeId ScopeId) const
{
	if (const TArray<TWeakObjectPtr<APlayerController>>* Players = ScopePlayerInterests.Find(ScopeId))
	{
		return Players->Num();
	}
	return 0;
}

void UScopedMessageSubsystem::NotifyScopeOccupancyChanged(FScopedMessageScopeId ScopeId)
{
	OnScopeOccupancyChanged.Broadcast(ScopeId, GetScopePlayerCount(ScopeId));
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
	// 失效清理（如玩家掉线）也是占用变化的来源。先收集变化的作用域及其新计数，
	// 待迭代结束后再广播，避免监听者在回调中修改 ScopePlayerInterests。
	TArray<TPair<FScopedMessageScopeId, int32>> PendingNotifications;

	for (auto It = ScopePlayerInterests.CreateIterator(); It; ++It)
	{
		const int32 NumRemoved = It.Value().RemoveAll([](const TWeakObjectPtr<APlayerController>& Player)
		{
			return !Player.IsValid();
		});

		if (NumRemoved > 0)
		{
			PendingNotifications.Emplace(It.Key(), It.Value().Num());
		}

		if (It.Value().Num() == 0)
		{
			It.RemoveCurrent();
		}
	}

	for (const TPair<FScopedMessageScopeId, int32>& Notification : PendingNotifications)
	{
		OnScopeOccupancyChanged.Broadcast(Notification.Key, Notification.Value);
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
			// Client RPCs on the PlayerController-owned bridge are owner-targeted, so
			// only the registered player receives this scoped packet.
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

	if (!ActorSpawnedDelegateHandles.Contains(World))
	{
		// Worlds can appear before their GameState/PlayerControllers are spawned, so
		// late actors are watched and given bridge components as they arrive.
		const FDelegateHandle SpawnedHandle = World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateUObject(this, &UScopedMessageSubsystem::OnActorSpawned));
		ActorSpawnedDelegateHandles.Add(World, SpawnedHandle);
	}

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

void UScopedMessageSubsystem::OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
	if (!World)
	{
		return;
	}

	if (const FDelegateHandle* SpawnedHandle = ActorSpawnedDelegateHandles.Find(World))
	{
		World->RemoveOnActorSpawnedHandler(*SpawnedHandle);
		ActorSpawnedDelegateHandles.Remove(World);
	}

	CleanupAllInvalidListeners();
	CleanupInvalidPlayerInterests();
	LocalScopeMap.Empty();
	for (auto It = EndPlayBoundActors.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			It.RemoveCurrent();
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

void UScopedMessageSubsystem::OnActorEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason)
{
	if (!Actor)
	{
		return;
	}

	Actor->OnEndPlay.RemoveDynamic(this, &UScopedMessageSubsystem::OnActorEndPlay);
	EndPlayBoundActors.Remove(Actor);
	CleanupInvalidListenersForOwner(Actor);
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
