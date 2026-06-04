# Scoped Message System

A lightweight, interface-driven, network-replicated local message routing system for Unreal Engine. It allows sending and receiving messages isolated within logical boundaries (Camps, Level Instances, Rooms) with zero configuration.

---

## Key Features

- **Scoped Isolation**: Messages are routed based on a combination of a channel tag and a scope context tag (`ScopeId`).
- **Zero-Code Context**: In Blueprints, the `ScopeContext` parameter is automatically wired to `Self`. The subsystem resolves the scope tag dynamically by crawling the caller's hierarchy.
- **Robust Network Replication**: Replicates message payloads from Server to Clients through a dynamically spawned component on the `GameState`, bypassing Net Relevancy issues.
- **Memory Safety (RAII)**: Built around `FInstancedStruct` (from the StructUtils plugin) for payload transmission. Eliminates custom thunks, raw memory allocators, and pointer casting.
- **Auto-Generated Tags**: Seamlessly creates unique dynamic tags for scope providers that do not supply a static tag.

---

## Quick Start

### 1. Declaring a Scope Provider
Implement the `IScopeContextProvider` interface on any Actor or object that defines a boundary (e.g. a Zone Manager, or Level Instance):

```cpp
#include "ScopeContextProvider.h"
#include "MyZoneManager.generated.h"

UCLASS()
class AMyZoneManager : public AActor, public IScopeContextProvider
{
    GENERATED_BODY()
public:
    virtual FGameplayTag GetScopeId() const override
    {
        return FGameplayTag::RequestGameplayTag("Scope.Zone.Alpha");
    }
};
```

### 2. Broadcasting Scoped Messages

#### In C++
```cpp
FMyPayload MessageData;
MessageData.MessageText = TEXT("Gate Opened");

UScopedMessageSubsystem& Subsystem = UScopedMessageSubsystem::Get(GetWorld());
Subsystem.BroadcastMessage(
    this, // WorldContextObject (acts as default scope context if ScopeContext is null)
    FGameplayTag::RequestGameplayTag("Event.DoorState"),
    MessageData,
    this, // Optional specific ScopeContext
    EScopedMessageReplication::ServerToAllClients
);
```

#### In Blueprints
Place the **Broadcast Scoped Message** node. The `Scope Context` pin is hidden by default and automatically bound to `Self`.

---

### 3. Listening for Scoped Messages

#### In C++
```cpp
UScopedMessageSubsystem& Subsystem = UScopedMessageSubsystem::Get(GetWorld());
FScopedMessageListenerHandle Handle = Subsystem.Subscribe<FMyPayload>(
    FGameplayTag::RequestGameplayTag("Event.DoorState"),
    this,
    &AMyActor::OnDoorStateChanged,
    this // Auto-resolves ScopeId
);

// Unregister when done
Handle.Unregister();
```

#### In Blueprints
Use the **Listen for Scoped Messages** async node. The `Scope Context` pin is automatically bound to `Self`, and the `Payload` is returned as a wild-card `FInstancedStruct` that you can break using standard Unreal Engine nodes.

---

## Installation
1. Clone this repository into your project's `Plugins/` folder.
2. Enable the **StructUtils** engine plugin in your `.uproject`.
3. Add `"ScopedMessageSystem"` to your game module's dependency list in `*.Build.cs`.

---

## Comparison with GameplayMessageRouter

| Feature / Dimension | GameplayMessageRouter (Native) | Scoped Message System (SMS) |
| :--- | :--- | :--- |
| **Routing Map** | Single-level: `Channel -> Listeners` | Dual-level: `ScopeId -> Channel -> Listeners` |
| **Isolation** | None (Global broadcasts only; causes crosstalk) | Isolated within local scopes (Global fallback if `ScopeId` is empty) |
| **Scope Resolving** | Not supported | Interface-driven (`IScopeContextProvider`) with $O(1)$ hierarchical automatic resolution |
| **Networking** | No built-in replication (Requires custom replication layer) | Built-in replication strategies (`LocalOnly`, `ServerToAllClients`, `ServerToScopedClients`) |
| **Net Relevancy Safety** | N/A | High (Replicator attached to `GameState` ensuring `bAlwaysRelevant` distribution) |
| **Dynamic Tag Safety** | N/A | High (Dynamic tags sent via `FName` instead of `FGameplayTag` network indices to prevent mismatch crashes) |
| **Memory Management** | Template type arguments / Raw memory operations | Modern C++ RAII using `FInstancedStruct` (StructUtils) for type safety and automatic alignment |
| **Blueprint Usability** | Custom compiler `K2Node` (requires separate uncooked/editor module) | Standard async node with `DefaultToSelf` and `FInstancedStruct` outputs (pure runtime, no editor modules required) |

