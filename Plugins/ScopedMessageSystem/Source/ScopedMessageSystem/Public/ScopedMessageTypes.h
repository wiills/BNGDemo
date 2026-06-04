#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ScopedMessageTypes.generated.h"

class UScopedMessageSubsystem;

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
	ServerToAllClients,
	ServerToScopedClients
};

USTRUCT(BlueprintType)
struct SCOPEDMESSAGESYSTEM_API FScopedMessageListenerHandle
{
	GENERATED_BODY()

public:
	FScopedMessageListenerHandle() = default;

	void Unregister();

	bool IsValid() const { return HandleID != 0; }

	FGameplayTag GetScopeId() const { return ScopeId; }
	FGameplayTag GetChannel() const { return Channel; }

private:
	friend class UScopedMessageSubsystem;

	UPROPERTY(Transient)
	TWeakObjectPtr<UScopedMessageSubsystem> Subsystem;

	UPROPERTY(Transient)
	FGameplayTag ScopeId;

	UPROPERTY(Transient)
	FGameplayTag Channel;

	UPROPERTY(Transient)
	int32 HandleID = 0;

	FScopedMessageListenerHandle(UScopedMessageSubsystem* InSubsystem, FGameplayTag InScopeId, FGameplayTag InChannel, int32 InID)
		: Subsystem(InSubsystem), ScopeId(InScopeId), Channel(InChannel), HandleID(InID) {}
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
