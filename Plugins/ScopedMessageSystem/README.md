# Scoped Message System

Scoped Message System is a Poi-focused message router for Unreal Engine. It lets
actors inside one Poi instance communicate with shared GameplayTag channels
without leaking those events into another Poi using the same template.

## Key Features

- `ScopeId + Channel` routing for Poi-local isolation.
- `AScopedMessagePoiActor` shared base for common ScopeId readiness handling.
- `AScopedMessagePoiRootActor` and `AScopedMessagePoiSubActor` base classes for
  common Poi setup.
- Replicated `FScopedMessageScopeId` instead of runtime GameplayTags for scope IDs.
- `UScopedMessageScopeComponent` for Poi root actors.
- Server-authoritative network modes, including `ServerToScopedClients`.
- PlayerController bridge components for owner-targeted scoped client delivery.
- C++ templates and Blueprint async listening through `FInstancedStruct`.

## Quick Start

### 1. Add a scope to the Poi root

Prefer deriving your Poi root from `AScopedMessagePoiRootActor`. It already owns
the replicated `UScopedMessageScopeComponent`, generates a runtime ScopeId on the
server by default, and can automatically register current players for scoped
client delivery during BeginPlay. You normally do not need to hand-author ScopeId
values on Poi roots.

Actors inside the Poi can derive from `AScopedMessagePoiSubActor`. The base class
waits until a valid ScopeId can be resolved locally, which avoids client BeginPlay
subscribing into the empty global scope before the root ScopeId replicates.
Both classes share `AScopedMessagePoiActor` underneath for common ScopeId readiness
and retry behavior.

For custom actor hierarchies, you can still add `UScopedMessageScopeComponent`
manually or implement `IScopeContextProvider`:

```cpp
UCLASS()
class AMyPoiInstance : public AActor, public IScopeContextProvider
{
	GENERATED_BODY()

public:
	virtual FScopedMessageScopeId GetScopeId_Implementation() const override
	{
		return PoiScopeId;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FScopedMessageScopeId PoiScopeId;
};
```

### 2. Broadcast inside the Poi

```cpp
FMyPayload Payload;
Payload.MessageText = TEXT("Terminal activated");

UScopedMessageSubsystem::Get(this).BroadcastMessage(
	this,
	FGameplayTag::RequestGameplayTag(TEXT("Poi.Terminal.Activated")),
	Payload,
	EScopedMessageReplication::ServerToScopedClients);
```

### 3. Listen inside the same Poi

```cpp
ListenerHandle = UScopedMessageSubsystem::Get(this).Subscribe<FMyPayload>(
	FGameplayTag::RequestGameplayTag(TEXT("Poi.Terminal.Activated")),
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

Unregister when the player leaves the Poi.

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

The codebase uses `Poi` in class names and demo GameplayTag channels, for example
`AScopedMessagePoiRootActor` and `Poi.Demo.Terminal.Activated`.

## Payload Model

Blueprint-facing APIs use `FInstancedStruct` so Blueprint graphs can work with a
real struct value instead of a raw byte envelope. The network layer still uses
`FScopedMessagePayload` internally (`StructPath + Bytes`) because RPCs need a
stable serialized representation.

The plugin uses `FInstancedStruct` from engine headers, but this project does not
declare an explicit StructUtils dependency in `.uplugin`, `.uproject`, or
`Build.cs`.

## Payload Helpers

C++ callers can wrap and decode reflected UStruct payloads through the network
envelope helpers:

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

## Tests And Demo

The source `Test` folder intentionally contains only:

- Poi demo actors and payload types for manual in-editor validation.
- Automation types and tests for payload encode/decode, scoped isolation, partial
  channel matching, and custom resolver override behavior.

The older generic broadcaster/listener/scope-provider manual sample was removed
so the plugin test surface stays aligned with Poi-style scoped communication.
