# Scoped Message POI Demo

This demo verifies that two identical POI instances can use the same message
channels without crosstalk.

## Actors

- `AScopedMessagePOIDemoRoot`: POI root with `UScopedMessageScopeComponent`.
- `AScopedMessagePOIDemoTerminal`: broadcasts `POI.Demo.Terminal.Activated`.
- `AScopedMessagePOIDemoDoor`: listens for terminal activation in the same scope.

## Setup

1. Place two `AScopedMessagePOIDemoRoot` actors in the level.
2. Under each root, place or attach one `AScopedMessagePOIDemoTerminal`.
3. Under each root, place or attach one `AScopedMessagePOIDemoDoor`.
4. Keep both terminals on channel `POI.Demo.Terminal.Activated`.
5. Keep both doors on channel `POI.Demo.Terminal.Activated`.
6. Set each door's `RequiredTerminalId` to match the terminal inside the same POI.

The important part is attachment or ownership: the terminal and door must resolve
the same POI root through the scope resolver.

## Network Setup

For `ServerToScopedClients`, register interested players on the server:

```cpp
DemoRoot->RegisterPlayer(PlayerController);
```

In a real POI system, call this when the player enters or streams in the POI, and
call `UnregisterPlayer` when the player leaves.

## Test

Call `ActivateTerminal` on the terminal in POI A.

Expected result:

- Door in POI A opens.
- Door in POI B stays closed.
- Logs show the same channel but different `ScopeId` values.

Call `ActivateTerminal` on the terminal in POI B.

Expected result:

- Door in POI B opens.
- Door in POI A is unaffected.
