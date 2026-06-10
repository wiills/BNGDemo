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
- C++ templates and Blueprint async listening through `FInstancedStruct`.

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

## Payload Model

Blueprint-facing APIs use `FInstancedStruct` so Blueprint graphs can work with a
real struct value instead of a raw byte envelope. The network layer still uses
`FScopedMessagePayload` internally (`StructPath + Bytes`) because RPCs need a
stable serialized representation.

The plugin uses the engine's StructUtils module in C++ for `FInstancedStruct`,
but it does not add a StructUtils plugin entry to `.uplugin` or `.uproject`.

## Payload Helpers

C++ callers can wrap and decode reflected UStruct payloads without StructUtils:

```cpp
FScopedMessagePayload Packed = FScopedMessagePayload::Make(MyPayload);

FMyPayload Decoded;
if (Packed.TryDecode(Decoded))
{
	// Use Decoded
}
```

Blueprint callers normally receive `FInstancedStruct` from listen nodes. If a
network envelope needs to be inspected or converted manually,
`UScopedMessagePayloadLibrary` provides conversion helpers between
`FScopedMessagePayload` and `FInstancedStruct`.

## Scope Resolver Extension

The default resolver checks the context object, actor components, component owner,
actor owner chain, attachment parent chain, and outer chain. Projects can register
a custom resolver before the default resolver:

```cpp
FDelegateHandle Handle = Subsystem.RegisterScopeResolver(
	FScopedMessageScopeResolver::CreateLambda(
		[](UObject* Context, FScopedMessageScopeId& OutScopeId)
		{
			// Return true after setting OutScopeId to override default resolution.
			return false;
		}));
```

Use `UnregisterScopeResolver(Handle)` when the resolver owner shuts down.

## Debugging

`DumpRoutingTable()` logs scopes, channels, and listener counts.
`DumpScopeResolution(Context)` logs custom resolver results followed by the
default resolver result.
