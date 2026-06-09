# Scoped Message System - POI Architecture

This plugin is a scoped message layer for POI-style gameplay spaces. It solves the
case where many identical POI instances contain actors that use the same channels
but must not hear each other's messages.

## Core Model

Messages are routed by two keys:

```text
FScopedMessageScopeId + FGameplayTag Channel -> ListenerList
```

- `FScopedMessageScopeId` is a replicated `FName` identifier for one POI or
  logical gameplay space. It is not a GameplayTag, because runtime scope IDs must
  be stable across server and clients.
- `Channel` remains a static GameplayTag such as `POI.Terminal.Activated` or
  `POI.Objective.Progressed`.
- An empty scope is treated as a global fallback and should be avoided for POI
  instance communication.

## Scope Ownership

The recommended setup is:

```text
APOIInstanceActor
  - UScopedMessageScopeComponent
      Replicated ScopeId

Actors inside the POI
  - owned by, attached to, outered under, or otherwise able to resolve the POI root
```

Scope resolution checks:

1. The context object itself.
2. Components on an actor context.
3. Component owner.
4. Actor owner chain.
5. Actor attachment parent chain.
6. UObject outer chain.

Any object or Blueprint can also implement `IScopeContextProvider` directly.

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
PlayerController. This avoids using global multicast for POI-local events and is
the path to prefer for mobile bandwidth.

## Player Interest

The server must register which players are interested in each scope:

```cpp
UScopedMessageSubsystem::Get(this).RegisterPlayerForScope(PlayerController, ScopeId);
```

Call this when a player enters, activates, streams in, or otherwise becomes
relevant to a POI. Unregister when the player leaves the POI.

Client-to-server messages are rejected unless the sending PlayerController is
registered for that scope.

## Payload Serialization

Network packets carry:

```text
ScopeId
Channel
PayloadStructPath
PayloadBytes
```

The payload struct is resolved by path on the receiver, then deserialized into an
`FStructOnScope` for local dispatch. Blueprint-facing async nodes expose the
serialized `FScopedMessagePayload` packet instead of `FInstancedStruct`, so this
plugin does not depend on StructUtils. Payloads should be plain reflected UStruct
data. Avoid raw UObject pointers inside payloads unless a project-specific
serializer is added. For authoritative game state, replicate state through normal
gameplay components; use scoped messages for triggers and notifications.

## POI Task Pattern

Use scoped messages for decoupled events:

```text
Terminal -> POI.Terminal.Activated
Door     -> listens POI.Terminal.Activated
Spawner  -> listens POI.Alert.Raised
ObjectiveStateComponent -> listens and owns authoritative state
UI       -> listens server/client notifications for the local player's POI
```

Messages are not the source of truth. The POI objective/state component should
own durable state and replicate that state normally.
