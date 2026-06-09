# Scoped Message System

Scoped Message System is a POI-focused message router for Unreal Engine. It lets
actors inside one POI instance communicate with shared GameplayTag channels
without leaking those events into another POI using the same template.

## Key Features

- `ScopeId + Channel` routing for POI-local isolation.
- Replicated `FScopedMessageScopeId` instead of runtime GameplayTags for scope IDs.
- `UScopedMessageScopeComponent` for POI root actors.
- Server-authoritative network modes, including `ServerToScopedClients`.
- PlayerController bridge components for owner-targeted scoped client delivery.
- C++ templates and Blueprint async listening through `FScopedMessagePayload`.

## Quick Start

### 1. Add a scope to the POI root

Add `UScopedMessageScopeComponent` to the actor that represents one POI instance.
Set `ScopeId` manually for authored POIs, or let the server generate one at
runtime and replicate it.

You can also implement `IScopeContextProvider`:

```cpp
UCLASS()
class AMyPOIInstance : public AActor, public IScopeContextProvider
{
	GENERATED_BODY()

public:
	virtual FScopedMessageScopeId GetScopeId_Implementation() const override
	{
		return POIScopeId;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FScopedMessageScopeId POIScopeId;
};
```

### 2. Broadcast inside the POI

```cpp
FMyPayload Payload;
Payload.MessageText = TEXT("Terminal activated");

UScopedMessageSubsystem::Get(this).BroadcastMessage(
	this,
	FGameplayTag::RequestGameplayTag(TEXT("POI.Terminal.Activated")),
	Payload,
	EScopedMessageReplication::ServerToScopedClients);
```

### 3. Listen inside the same POI

```cpp
ListenerHandle = UScopedMessageSubsystem::Get(this).Subscribe<FMyPayload>(
	FGameplayTag::RequestGameplayTag(TEXT("POI.Terminal.Activated")),
	this,
	&AMyActor::OnTerminalActivated);
```

Unregister handles in `EndPlay` when the listener owns the lifetime:

```cpp
ListenerHandle.Unregister();
```

### 4. Register player interest on the server

`ServerToScopedClients` only sends to players registered for that scope.

```cpp
UScopedMessageSubsystem::Get(this).RegisterPlayerForScope(PlayerController, ScopeId);
```

Unregister when the player leaves the POI.

## Network Modes

| Mode | Behavior |
| :--- | :--- |
| `LocalOnly` | Dispatches only in the current process. |
| `ServerOnly` | Dispatches only on authority or standalone. |
| `ClientToServer` | Sends through the local PlayerController bridge; server validates scope interest. |
| `ServerToAllClients` | Dispatches on server and multicasts to every client. |
| `ServerToScopedClients` | Dispatches on server and sends only to registered players for the scope. |

## Notes

Payloads sent over the network should be plain reflected UStruct data. Scoped
messages are for triggers and notifications; long-lived mission state should live
in authoritative replicated components.

The plugin does not depend on StructUtils. Blueprint listeners receive serialized
`FScopedMessagePayload` packets; typed struct access should be handled in C++ or
with a project-specific Blueprint helper for known payload types.
