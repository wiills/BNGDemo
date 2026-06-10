# Scoped Message System - Poi Architecture

This plugin is a scoped message layer for Poi-style gameplay spaces. It solves the
case where many identical Poi instances contain actors that use the same channels
but must not hear each other's messages.

## Core Model

Messages are routed by two keys:

```text
FScopedMessageScopeId + FGameplayTag Channel -> ListenerList
```

- `FScopedMessageScopeId` is a replicated `FName` identifier for one Poi or
  logical gameplay space. It is not a GameplayTag, because runtime scope IDs must
  be stable across server and clients.
- `Channel` remains a static GameplayTag such as `Poi.Terminal.Activated` or
  `Poi.Objective.Progressed`.
- An empty scope is treated as a global fallback and should be avoided for Poi
  instance communication.

## Scope Ownership

The recommended setup is:

```text
AScopedMessagePoiRootActor
  - UScopedMessageScopeComponent
      Replicated ScopeId
  - optional auto player-interest registration

AScopedMessagePoiSubActor
  - waits until local ScopeId resolution is valid
  - owned by, attached to, outered under, or otherwise able to resolve the Poi root
```

Projects can inherit these two base classes for the common case. Lower-level
systems can still use `UScopedMessageScopeComponent`, `IScopeContextProvider`, or
custom scope resolvers directly.

Scope resolution checks:

1. The context object itself.
2. Components on an actor context.
3. Component owner.
4. Actor owner chain.
5. Actor attachment parent chain.
6. UObject outer chain.

Any object or Blueprint can also implement `IScopeContextProvider` directly.
Projects can also register C++ scope resolver delegates. Custom resolvers run
before the default resolver and can map project-specific objects to stable scope
IDs without changing the plugin's default traversal rules.

## Local Routing

`UScopedMessageSubsystem` stores listeners in a per-GameInstance routing table:

```text
TMap<FScopedMessageScopeId, TMap<FGameplayTag, ListenerList>>
```

`ExactMatch` listeners only receive their registered channel. `PartialMatch`
listeners receive child channels when the broadcast channel matches the registered
parent tag.

## Network Strategy

The system is server-authoritative by default and supports explicit replication
intent:

- `LocalOnly`: dispatch only in the current process.
- `ServerOnly`: dispatch only on authority or standalone.
- `ClientToServer`: serialize payload and send it through the local
  PlayerController bridge.
- `ServerToAllClients`: dispatch on server and multicast through the GameState
  replicator to every client.
- `ServerToScopedClients`: dispatch on server and send owner-targeted client RPCs
  only to PlayerControllers registered for the target scope.

Scoped client delivery uses `UScopedMessageClientBridgeComponent` on each
PlayerController. This avoids using global multicast for Poi-local events and is
the path to prefer for mobile bandwidth.

## Player Interest

The server must register which players are interested in each scope:

```cpp
UScopedMessageSubsystem::Get(this).RegisterPlayerForScope(PlayerController, ScopeId);
```

Call this when a player enters, activates, streams in, or otherwise becomes
relevant to a Poi. Unregister when the player leaves the Poi.

Client-to-server messages are rejected unless the sending PlayerController is
registered for that scope.

`AScopedMessagePoiRootActor` provides `RegisterPlayer`, `UnregisterPlayer`,
`RegisterAllCurrentPlayers`, and `UnregisterAllCurrentPlayers`. Its BeginPlay
auto-registration is mainly a convenience for demos and simple Pois; real
streaming or interest-management systems should still call these functions when
players enter and leave a Poi.

## Payload Serialization

Network packets carry:

```text
ScopeId
Channel
PayloadStructPath
PayloadBytes
```

The network envelope carries `FScopedMessagePayload` (`StructPath + Bytes`). The
payload struct is resolved by path on the receiver, then deserialized into an
`FStructOnScope` for local C++ dispatch. Blueprint-facing APIs use
`FInstancedStruct` so graphs can work with real struct values instead of raw
bytes.

The plugin uses `FInstancedStruct` from engine headers for Blueprint-facing APIs,
but this project does not declare an explicit StructUtils dependency in
`.uplugin`, `.uproject`, or `Build.cs`. Payloads should be plain reflected
UStruct data. Avoid raw UObject pointers inside payloads unless a project-specific
serializer is added. For authoritative game state, replicate state through normal
gameplay components; use scoped messages for triggers and notifications.

`FScopedMessagePayload::Make<T>` and `TryDecode<T>` cover C++ typed encode/decode.
`UScopedMessagePayloadLibrary` exposes generic Blueprint inspection and conversion
helpers between `FScopedMessagePayload` and `FInstancedStruct`.

## Diagnostics

The subsystem exposes two lightweight dump helpers:

- `DumpRoutingTable()`: logs scope, channel, and listener counts.
- `DumpScopeResolution(Context)`: logs custom resolver output and default resolver
  output for one context object.

Automation coverage currently includes payload encode/decode, scope isolation,
partial channel matching, and custom resolver override behavior. The manual test
surface is the Poi demo actors under `Source/ScopedMessageSystem/*/Test`; the old
generic broadcaster/listener/scope-provider sample was removed to avoid carrying
two competing examples.

## Poi Task Pattern

Use scoped messages for decoupled events:

```text
Terminal -> Poi.Terminal.Activated
Door     -> listens Poi.Terminal.Activated
Spawner  -> listens Poi.Alert.Raised
ObjectiveStateComponent -> listens and owns authoritative state
UI       -> listens server/client notifications for the local player's Poi
```

Messages are not the source of truth. The Poi objective/state component should
own durable state and replicate that state normally.
