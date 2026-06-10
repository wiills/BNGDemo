#include "ScopedMessageTypes.h"
#include "ScopedMessageSubsystem.h"

const UScriptStruct* FScopedMessagePayload::ResolvePayloadType() const
{
	if (PayloadStructPath.IsEmpty())
	{
		return nullptr;
	}

	if (UScriptStruct* Struct = FindObject<UScriptStruct>(nullptr, *PayloadStructPath))
	{
		return Struct;
	}

	return LoadObject<UScriptStruct>(nullptr, *PayloadStructPath);
}

bool FScopedMessagePayload::IsPayloadOfType(const UScriptStruct* PayloadType) const
{
	return PayloadType && ResolvePayloadType() == PayloadType;
}

FScopedMessageListenerHandle::FScopedMessageListenerHandle(
	UScopedMessageSubsystem* InSubsystem,
	FScopedMessageScopeId InScopeId,
	FGameplayTag InChannel,
	int32 InID)
	: Subsystem(InSubsystem)
	, ScopeId(InScopeId)
	, Channel(InChannel)
	, HandleID(InID)
{
}

void FScopedMessageListenerHandle::Unregister()
{
	if (UScopedMessageSubsystem* StrongSubsystem = Subsystem.Get())
	{
		StrongSubsystem->Unsubscribe(*this);
	}
}
