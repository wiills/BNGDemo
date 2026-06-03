#include "ScopedMessageSubsystem.h"
#include "ScopeContextProvider.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

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

void UScopedMessageSubsystem::Deinitialize()
{
	RoutingTable.Empty();
	Super::Deinitialize();
}

FGameplayTag UScopedMessageSubsystem::ResolveScopeId(UObject* ScopeContext) const
{
	if (!ScopeContext)
	{
		return FGameplayTag::EmptyTag;
	}

	if (const IScopeContextProvider* Provider = Cast<IScopeContextProvider>(ScopeContext))
	{
		const FGameplayTag Tag = Provider->GetScopeId();
		if (Tag.IsValid())
		{
			return Tag;
		}
	}

	UObject* Current = ScopeContext;
	while (Current)
	{
		if (const IScopeContextProvider* Provider = Cast<IScopeContextProvider>(Current))
		{
			const FGameplayTag Tag = Provider->GetScopeId();
			if (Tag.IsValid())
			{
				return Tag;
			}
		}
		Current = Current->GetOuter();
	}

	return FGameplayTag::EmptyTag;
}

void UScopedMessageSubsystem::BroadcastMessageInternal(
	FGameplayTag Channel,
	const UScriptStruct* PayloadType,
	const void* PayloadBytes,
	FGameplayTag ScopeId,
	EScopedMessageReplication Replication)
{
	if (!Channel.IsValid())
	{
		UE_LOG(LogScopedMessageSubsystem, Warning, TEXT("BroadcastMessageInternal called with invalid Channel"));
		return;
	}

	if (!PayloadType || !PayloadBytes)
	{
		UE_LOG(LogScopedMessageSubsystem, Warning, TEXT("BroadcastMessageInternal called with null PayloadType or PayloadBytes"));
		return;
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

	TArray<FScopedMessageListenerData> ListenersCopy = ListenerList->Listeners;
	for (FScopedMessageListenerData& Listener : ListenersCopy)
	{
		if (!Listener.Owner.IsValid())
		{
			continue;
		}

		if (Listener.PayloadType.IsValid() && Listener.PayloadType.Get() != PayloadType)
		{
			UE_LOG(LogScopedMessageSubsystem, Warning,
				TEXT("Payload type mismatch for channel %s: expected %s, got %s"),
				*Channel.ToString(),
				*GetNameSafe(Listener.PayloadType.Get()),
				*GetNameSafe(PayloadType));
			continue;
		}

		Listener.Callback(Channel, PayloadType, PayloadBytes);
	}

	if (Replication != EScopedMessageReplication::LocalOnly)
	{
		TArray<uint8> PayloadBuffer;
		FMemoryWriter Writer(PayloadBuffer);
		const_cast<UScriptStruct*>(PayloadType)->SerializeItem(Writer, const_cast<void*>(PayloadBytes), nullptr);

		NetMulticast_BroadcastMessage(Channel, ScopeId, PayloadType, PayloadBuffer);
	}
}

FScopedMessageListenerHandle UScopedMessageSubsystem::SubscribeInternal(
	FGameplayTag Channel,
	TFunction<void(FGameplayTag, const UScriptStruct*, const void*)>&& Callback,
	const UScriptStruct* PayloadType,
	FGameplayTag ScopeId,
	EScopedMessageMatch MatchType,
	UObject* Owner)
{
	if (!Channel.IsValid())
	{
		UE_LOG(LogScopedMessageSubsystem, Warning, TEXT("SubscribeInternal called with invalid Channel"));
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

void UScopedMessageSubsystem::UnsubscribeInternal(FGameplayTag ScopeId, FGameplayTag Channel, int32 HandleID)
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

void UScopedMessageSubsystem::CleanupInvalidListeners(FGameplayTag ScopeId, FGameplayTag Channel)
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

void UScopedMessageSubsystem::NetMulticast_BroadcastMessage_Implementation(
	FGameplayTag Channel,
	FGameplayTag ScopeId,
	const UScriptStruct* PayloadType,
	const TArray<uint8>& PayloadBytes)
{
	if (!PayloadType || PayloadBytes.Num() == 0)
	{
		return;
	}

	FMemoryReader Reader(PayloadBytes);
	UScriptStruct* NonConstPayloadType = const_cast<UScriptStruct*>(PayloadType);
	uint8* PayloadData = static_cast<uint8*>(FMemory::Malloc(NonConstPayloadType->GetStructureSize()));
	NonConstPayloadType->InitializeStruct(PayloadData);
	NonConstPayloadType->SerializeItem(Reader, PayloadData, nullptr);

	BroadcastMessageInternal(Channel, NonConstPayloadType, PayloadData, ScopeId, EScopedMessageReplication::LocalOnly);

	NonConstPayloadType->DestroyStruct(PayloadData);
	FMemory::Free(PayloadData);
}

void UScopedMessageSubsystem::execK2_BroadcastMessage(UObject* Context, FFrame& Stack, void* const Z_Param__Result)
{
	P_GET_STRUCT(FGameplayTag, Channel);

	Stack.MostRecentPropertyAddress = nullptr;
	Stack.MostRecentPropertyContainer = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);

	void* MessagePtr = Stack.MostRecentPropertyAddress;
	FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);

	P_GET_OBJECT(UObject, ScopeContext);
	P_GET_ENUM(EScopedMessageReplication, Replication);

	P_FINISH;

	if (!StructProp || !MessagePtr)
	{
		UE_LOG(LogScopedMessageSubsystem, Warning, TEXT("K2_BroadcastMessage: invalid message struct"));
		return;
	}

	UScopedMessageSubsystem* Subsystem = Cast<UScopedMessageSubsystem>(Context);
	if (!Subsystem)
	{
		return;
	}

	const UScriptStruct* PayloadType = StructProp->Struct;
	Subsystem->BroadcastMessageInternal(Channel, PayloadType, MessagePtr, Subsystem->ResolveScopeId(ScopeContext), Replication);
}
