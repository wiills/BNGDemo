#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ScopedMessageTypes.generated.h"

class UScopedMessageSubsystem;

/**
 * Stable network identifier for a logical message scope, such as one POI instance.
 *
 * Channel names remain GameplayTags. Scope IDs are intentionally FNames so runtime POI
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
	ExactMatch   UMETA(DisplayName = "Exact Match"),
	PartialMatch UMETA(DisplayName = "Partial Match (includes children)")
};

UENUM(BlueprintType)
enum class EScopedMessageReplication : uint8
{
	LocalOnly,
	ServerOnly,
	ClientToServer,
	ServerToAllClients,
	ServerToScopedClients
};

USTRUCT(BlueprintType)
struct SCOPEDMESSAGESYSTEM_API FScopedMessagePayload
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Scoped Message")
	FString PayloadStructPath;

	UPROPERTY(BlueprintReadOnly, Category = "Scoped Message")
	TArray<uint8> PayloadBytes;

	bool IsValid() const
	{
		return !PayloadStructPath.IsEmpty() && PayloadBytes.Num() > 0;
	}
};

USTRUCT(BlueprintType)
struct SCOPEDMESSAGESYSTEM_API FScopedMessageNetworkPacket
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Scoped Message")
	FScopedMessageScopeId ScopeId;

	UPROPERTY(BlueprintReadOnly, Category = "Scoped Message")
	FGameplayTag Channel;

	UPROPERTY(BlueprintReadOnly, Category = "Scoped Message")
	FScopedMessagePayload Payload;
};

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

	TFunction<void(FGameplayTag Channel, const UScriptStruct*, const void*)> Callback;

	int32 HandleID = 0;
	EScopedMessageMatch MatchType = EScopedMessageMatch::ExactMatch;
	TWeakObjectPtr<UObject> Owner;
	TWeakObjectPtr<const UScriptStruct> PayloadType;
};
