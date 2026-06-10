#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "UObject/UObjectGlobals.h"
#include "ScopedMessageTypes.generated.h"

class UScopedMessageSubsystem;

/**
 * Stable network identifier for a logical message scope, such as one Poi instance.
 *
 * Channel names remain GameplayTags. Scope IDs are intentionally FNames so runtime Poi
 * instances can use replicated IDs without depending on per-process GameplayTag tables.
 */
USTRUCT(BlueprintType)
struct SCOPEDMESSAGESYSTEM_API FScopedMessageScopeId
{
	GENERATED_BODY()

public:
	FScopedMessageScopeId() = default;
	explicit FScopedMessageScopeId(FName InName) : Name(InName) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message")
	FName Name = NAME_None;

	bool IsValid() const { return !Name.IsNone(); }
	FString ToString() const { return Name.ToString(); }

	bool operator==(const FScopedMessageScopeId& Other) const
	{
		return Name == Other.Name;
	}

	bool operator!=(const FScopedMessageScopeId& Other) const
	{
		return !(*this == Other);
	}

	static FScopedMessageScopeId FromGameplayTag(FGameplayTag Tag)
	{
		return FScopedMessageScopeId(Tag.GetTagName());
	}
};

FORCEINLINE uint32 GetTypeHash(const FScopedMessageScopeId& ScopeId)
{
	return GetTypeHash(ScopeId.Name);
}

UENUM(BlueprintType)
enum class EScopedMessageMatch : uint8
{
	/** Receive only broadcasts whose channel exactly equals the registered channel. */
	ExactMatch   UMETA(DisplayName = "Exact Match"),

	/** Receive broadcasts on the registered channel and any child GameplayTags. */
	PartialMatch UMETA(DisplayName = "Partial Match (includes children)")
};

/**
 * Network intent for a scoped message broadcast.
 *
 * The enum is explicit so call sites choose whether a message is local gameplay
 * glue, client input, or server-authoritative notification. Routing is always
 * still keyed by ScopeId + Channel.
 */
UENUM(BlueprintType)
enum class EScopedMessageReplication : uint8
{
	/** Dispatch only in the current process. */
	LocalOnly,

	/** Dispatch only when running on authority or standalone. */
	ServerOnly,

	/** Client sends the packet to the server; the server validates player interest. */
	ClientToServer,

	/** Server dispatches locally and multicasts the packet to every client. */
	ServerToAllClients,

	/** Server dispatches locally and sends the packet only to players registered for the ScopeId. */
	ServerToScopedClients
};

/**
 * Serializable payload envelope used by network RPCs.
 *
 * Blueprint and local C++ paths work with typed UStruct memory or FInstancedStruct.
 * Network paths need a stable representation, so the payload is stored as a
 * reflected struct path plus serialized bytes.
 */
USTRUCT(BlueprintType)
struct SCOPEDMESSAGESYSTEM_API FScopedMessagePayload
{
	GENERATED_BODY()

	/** Full object path of the UScriptStruct used to serialize PayloadBytes. */
	UPROPERTY(BlueprintReadOnly, Category = "Scoped Message")
	FString PayloadStructPath;

	/** Serialized bytes produced by UScriptStruct::SerializeItem. */
	UPROPERTY(BlueprintReadOnly, Category = "Scoped Message")
	TArray<uint8> PayloadBytes;

	bool IsValid() const
	{
		return !PayloadStructPath.IsEmpty() && PayloadBytes.Num() > 0;
	}

	const UScriptStruct* ResolvePayloadType() const;
	bool IsPayloadOfType(const UScriptStruct* PayloadType) const;

	template <typename FMessageStruct>
	static FScopedMessagePayload Make(const FMessageStruct& Payload)
	{
		FScopedMessagePayload Result;
		const UScriptStruct* StructType = FMessageStruct::StaticStruct();
		Result.PayloadStructPath = StructType->GetPathName();

		FMemoryWriter Writer(Result.PayloadBytes);
		const_cast<UScriptStruct*>(StructType)->SerializeItem(Writer, const_cast<FMessageStruct*>(&Payload), nullptr);
		return Result;
	}

	template <typename FMessageStruct>
	bool TryDecode(FMessageStruct& OutPayload) const
	{
		const UScriptStruct* ExpectedType = FMessageStruct::StaticStruct();
		if (!IsPayloadOfType(ExpectedType))
		{
			return false;
		}

		FMemoryReader Reader(PayloadBytes);
		const_cast<UScriptStruct*>(ExpectedType)->SerializeItem(Reader, &OutPayload, nullptr);
		return !Reader.IsError();
	}
};

USTRUCT(BlueprintType)
struct SCOPEDMESSAGESYSTEM_API FScopedMessageNetworkPacket
{
	GENERATED_BODY()

	/** Logical scope that isolates otherwise identical GameplayTag channels. */
	UPROPERTY(BlueprintReadOnly, Category = "Scoped Message")
	FScopedMessageScopeId ScopeId;

	/** Static message channel, usually authored as a project GameplayTag. */
	UPROPERTY(BlueprintReadOnly, Category = "Scoped Message")
	FGameplayTag Channel;

	/** Serialized payload envelope for RPC transport. */
	UPROPERTY(BlueprintReadOnly, Category = "Scoped Message")
	FScopedMessagePayload Payload;
};

/**
 * Lightweight subscription token returned by Subscribe calls.
 *
 * Handles are safe to store by value. Calling Unregister removes the listener
 * from the subsystem if the subsystem is still alive.
 */
USTRUCT(BlueprintType)
struct SCOPEDMESSAGESYSTEM_API FScopedMessageListenerHandle
{
	GENERATED_BODY()

public:
	FScopedMessageListenerHandle() = default;

	void Unregister();

	bool IsValid() const { return HandleID != 0; }

	FScopedMessageScopeId GetScopeId() const { return ScopeId; }
	FGameplayTag GetChannel() const { return Channel; }

private:
	friend class UScopedMessageSubsystem;

	UPROPERTY(Transient)
	TWeakObjectPtr<UScopedMessageSubsystem> Subsystem;

	UPROPERTY(Transient)
	FScopedMessageScopeId ScopeId;

	UPROPERTY(Transient)
	FGameplayTag Channel;

	UPROPERTY(Transient)
	int32 HandleID = 0;

	FScopedMessageListenerHandle(UScopedMessageSubsystem* InSubsystem, FScopedMessageScopeId InScopeId, FGameplayTag InChannel, int32 InID);
};

USTRUCT()
struct FScopedMessageListenerData
{
	GENERATED_BODY()

	/** Type-erased callback used by both typed C++ and Blueprint async listeners. */
	TFunction<void(FGameplayTag Channel, const UScriptStruct*, const void*)> Callback;

	int32 HandleID = 0;
	EScopedMessageMatch MatchType = EScopedMessageMatch::ExactMatch;

	/** Weak owner prevents listeners from keeping actors alive. */
	TWeakObjectPtr<UObject> Owner;

	/** Optional expected payload type. Mismatched broadcasts are ignored with a warning. */
	TWeakObjectPtr<const UScriptStruct> PayloadType;
};
