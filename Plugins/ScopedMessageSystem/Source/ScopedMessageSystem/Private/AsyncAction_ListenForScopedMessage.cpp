#include "AsyncAction_ListenForScopedMessage.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "ScopedMessageSubsystem.h"
#include "UObject/ScriptMacros.h"
#include "UObject/Stack.h"

UAsyncAction_ListenForScopedMessage* UAsyncAction_ListenForScopedMessage::ListenForScopedMessages(
	UObject* WorldContextObject,
	FGameplayTag Channel,
	UScriptStruct* PayloadType,
	EScopedMessageMatch MatchType)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return nullptr;
	}

	UAsyncAction_ListenForScopedMessage* Action = NewObject<UAsyncAction_ListenForScopedMessage>();
	Action->WorldPtr = World;
	Action->ChannelToRegister = Channel;
	Action->MessageStructType = PayloadType;
	// Use WorldContextObject directly to resolve the scope context
	Action->ScopeContextObject = WorldContextObject;
	Action->MessageMatchType = MatchType;
	Action->RegisterWithGameInstance(World);

	return Action;
}

void UAsyncAction_ListenForScopedMessage::Activate()
{
	UWorld* World = WorldPtr.Get();
	if (!World)
	{
		SetReadyToDestroy();
		return;
	}

	if (!UScopedMessageSubsystem::HasInstance(World))
	{
		SetReadyToDestroy();
		return;
	}

	UScopedMessageSubsystem& Subsystem = UScopedMessageSubsystem::Get(World);

	UObject* ResolvedScopeContext = ScopeContextObject.Get();
	FScopedMessageScopeId ScopeId = Subsystem.ResolveScopeId(ResolvedScopeContext);

	TWeakObjectPtr<UAsyncAction_ListenForScopedMessage> WeakThis(this);
	ListenerHandle = Subsystem.SubscribeInternal(
		ChannelToRegister,
		[WeakThis](FGameplayTag ActualChannel, const UScriptStruct* StructType, const void* PayloadBytes)
		{
			if (UAsyncAction_ListenForScopedMessage* StrongThis = WeakThis.Get())
			{
				StrongThis->HandleMessageReceived(ActualChannel, StructType, PayloadBytes);
			}
		},
		MessageStructType.Get(),
		ScopeId,
		MessageMatchType,
		this
	);
}

void UAsyncAction_ListenForScopedMessage::SetReadyToDestroy()
{
	if (ListenerHandle.IsValid())
	{
		ListenerHandle.Unregister();
	}

	Super::SetReadyToDestroy();
}

bool UAsyncAction_ListenForScopedMessage::GetPayload(int32& OutPayload)
{
	checkNoEntry();
	return false;
}

DEFINE_FUNCTION(UAsyncAction_ListenForScopedMessage::execGetPayload)
{
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	void* MessagePtr = Stack.MostRecentPropertyAddress;
	FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);
	P_FINISH;

	bool bSuccess = false;
	if ((StructProp != nullptr) &&
		(StructProp->Struct != nullptr) &&
		(MessagePtr != nullptr) &&
		(StructProp->Struct == P_THIS->MessageStructType.Get()) &&
		(P_THIS->ReceivedMessagePayloadPtr != nullptr))
	{
		StructProp->Struct->CopyScriptStruct(MessagePtr, P_THIS->ReceivedMessagePayloadPtr);
		bSuccess = true;
	}

	*(bool*)RESULT_PARAM = bSuccess;
}

void UAsyncAction_ListenForScopedMessage::HandleMessageReceived(FGameplayTag Channel, const UScriptStruct* StructType, const void* PayloadBytes)
{
	if (!MessageStructType.IsValid() || (MessageStructType.Get() == StructType))
	{
		ReceivedMessagePayloadPtr = PayloadBytes;
		FScopedMessageScopeId ActualScopeId = ListenerHandle.IsValid() ? ListenerHandle.GetScopeId() : FScopedMessageScopeId();

		OnMessageReceived.Broadcast(this, Channel, ActualScopeId);

		ReceivedMessagePayloadPtr = nullptr;
	}

	if (!OnMessageReceived.IsBound())
	{
		SetReadyToDestroy();
	}
}
