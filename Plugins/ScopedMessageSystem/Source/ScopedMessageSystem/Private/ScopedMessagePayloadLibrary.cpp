#include "ScopedMessagePayloadLibrary.h"

#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"

bool UScopedMessagePayloadLibrary::IsPayloadValid(const FScopedMessagePayload& Payload)
{
	return Payload.IsValid();
}

bool UScopedMessagePayloadLibrary::IsPayloadOfType(const FScopedMessagePayload& Payload, const UScriptStruct* PayloadType)
{
	return Payload.IsPayloadOfType(PayloadType);
}

FString UScopedMessagePayloadLibrary::GetPayloadStructPath(const FScopedMessagePayload& Payload)
{
	return Payload.PayloadStructPath;
}

FName UScopedMessagePayloadLibrary::GetPayloadStructName(const FScopedMessagePayload& Payload)
{
	const UScriptStruct* PayloadType = Payload.ResolvePayloadType();
	return PayloadType ? PayloadType->GetFName() : NAME_None;
}

int32 UScopedMessagePayloadLibrary::GetPayloadSize(const FScopedMessagePayload& Payload)
{
	return Payload.PayloadBytes.Num();
}

bool UScopedMessagePayloadLibrary::MakePayloadFromInstancedStruct(const FInstancedStruct& InstancedStruct, FScopedMessagePayload& OutPayload)
{
	if (!InstancedStruct.IsValid())
	{
		OutPayload = FScopedMessagePayload();
		return false;
	}

	const UScriptStruct* PayloadType = InstancedStruct.GetScriptStruct();
	OutPayload.PayloadStructPath = PayloadType->GetPathName();
	OutPayload.PayloadBytes.Reset();

	FMemoryWriter Writer(OutPayload.PayloadBytes);
	const void* SourceMemory = InstancedStruct.GetMemory();
	const_cast<UScriptStruct*>(PayloadType)->SerializeItem(Writer, const_cast<void*>(SourceMemory), nullptr);
	return true;
}

bool UScopedMessagePayloadLibrary::MakeInstancedStructFromPayload(const FScopedMessagePayload& Payload, FInstancedStruct& OutInstancedStruct)
{
	const UScriptStruct* PayloadType = Payload.ResolvePayloadType();
	if (!PayloadType || Payload.PayloadBytes.Num() == 0)
	{
		OutInstancedStruct.Reset();
		return false;
	}

	OutInstancedStruct.InitializeAs(PayloadType);
	FMemoryReader Reader(Payload.PayloadBytes);
	const_cast<UScriptStruct*>(PayloadType)->SerializeItem(Reader, OutInstancedStruct.GetMutableMemory(), nullptr);
	return !Reader.IsError();
}
