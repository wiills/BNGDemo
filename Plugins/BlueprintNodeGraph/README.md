<div align="center">

[English](./README.md) | [中文](./README_CN.md)

---

</div>

# BlueprintNodeGraph
## UE5 Lightweight Parallel Flow Orchestration Framework (Pure Blueprint)
### Production-Ready · Battle-tested in Shipped Games

<img width="1378" height="750" alt="Image" src="https://github.com/user-attachments/assets/c583910a-0759-49f1-b9b1-b1df0bd5e71c" />

BlueprintNodeGraph is a **Blueprint-native async flow framework** designed specifically for UE5. It replaces scattered Delay/Wait nodes and simplifies task/story/level logic development, **supporting full-level logic validation in shipped projects**.

Core advantages: **Direct use in standard Blueprints, native parallel async, GC-safe, network replication, low overhead and easy extensibility**.

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Unreal Engine](https://img.shields.io/badge/unreal--engine-5.7+-green.svg)](https://www.unrealengine.com/)
[![Version](https://img.shields.io/badge/version-1.1-orange.svg)](https://github.com/your-repo)

## Table of Contents

- [Introduction](#introduction)
- [Why Choose BlueprintNodeGraph?](#why-choose-blueprintnodegraph)
- [Features](#features)
- [Quick Start](#quick-start)
- [Documentation](#documentation)
- [Examples](#examples)
- [Support](#support)
- [Contributing](#contributing)
- [License](#license)

## Introduction

BlueprintNodeGraph is a powerful Unreal Engine plugin that provides a rich set of async Blueprint nodes and delayed task execution system. This plugin enables developers to easily implement complex async logic in Blueprints without writing a lot of C++ code.

**Main Design Goals:**
- Intuitive and easy-to-use async nodes
- Complete task lifecycle management
- GC-safe object management
- Network replication support
- High performance, low overhead

**Core Files:**
- [ExLatentTaskBase.h](Source/BlueprintNodeGraph/Public/BlueprintTool/ExLatentTaskBase.h) - Latent task base class
- [ExLatentActionManager.h](Source/BlueprintNodeGraph/Public/BlueprintTool/ExLatentActionManager.h) - Delegate manager
- [ExK2Node_LatentTaskCall.h](Source/BlueprintNodeGraphEditor/Public/BlueprintTool/ExK2Node_LatentTaskCall.h) - K2 node extension

## Why Choose BlueprintNodeGraph?

### 🆚 Comparison with Other Solutions

| Feature | **BlueprintNodeGraph** | FlowGraph | StateTree |
|---------|----------------------|-----------|-----------|
| **Core Purpose** | Pure Blueprint async node system | Visual node programming | State machine system |
| **Learning Curve** | ✅ **Extremely Low** (like using existing nodes) | High (need to learn specific concepts) | High (need to learn state machine concepts) |
| **Usage Method** | ✅ **Drag nodes directly in standard Blueprint Editor** | Need to open separate FlowGraph Editor | Need to open specialized Editor |
| **Async Execution** | ✅ **Native async support** | Need additional implementation | Mainly synchronous |
| **Timeout Mechanism** | ✅ **Built-in timeout protection** | Need to implement manually | Need to implement manually |
| **GC Protection** | ✅ **Automatic GC-safe** | Need manual management | Need manual management |
| **Network Replication** | ✅ **Direct support** | Need to implement manually | Need to implement manually |
| **Latent Loop** | ✅ **Loop Delay and other ready-to-use nodes** | Need to combine nodes | Not applicable |
| **Multi-branch Sync** | ✅ **Wait Branches and other ready-to-use nodes** | Need to build manually | Need to combine state transitions |

### 💡 Our Core Advantages

#### 1. Works Directly in Standard Blueprint Editor

**Other Solutions:**
- FlowGraph requires a separate editor
- StateTree requires a specialized editor

**Our Solution:**
- ✅ Drag nodes directly in standard Blueprint Editor
- ✅ No need to learn a new editor
- ✅ Fully compatible with your existing workflow
- ✅ Learning cost is virtually zero

#### 2. Out-of-the-box Async Execution

**File Location:** [ExLatentProxyDefine.h](Source/BlueprintNodeGraph/Public/BlueprintTool/ExLatentProxyDefine.h)

```cpp
// Just drag and use:
Loop Delay (Duration: 1.0)
    ↓
Execute your logic
```

No complex chart configuration needed, just use it!

#### 3. Automatic GC Protection Mechanism

**File Location:** [ExAsyncActionBase.h#L96-113](Source/BlueprintNodeGraph/Public/BlueprintTool/ExAsyncActionBase.h#L96-113)

```cpp
// Automatic object lifecycle handling:
RegisterWithGameInstance(WorldContext);  // Prevent collection
SetReadyToDestroy();                    // Mark for destruction when complete
```

- ✅ Automatic management, no manual cleanup
- ✅ No memory leak risk

#### 4. Timeout Protection Mechanism

**File Location:** [ExLatentNodeInfo](Source/BlueprintNodeGraph/Public/BlueprintTool/ExLatentProxyDefine.h)

```
Node Properties → Configure Time Out: 10.0
    ↓
Automatic timeout protection
```

Prevents infinite waiting!

#### 5. Complete Task Lifecycle

**File Location:** [ExLatentTaskBase.h#L61](Source/BlueprintNodeGraph/Public/BlueprintTool/ExLatentTaskBase.h#L61)

```
CreateTask() → Activate() → Receive On Start → Receive On Stop → Destroy
```

- Complete state machine management
- Network replication sync support

### 🎯 Use Case Comparison

| Scenario | BlueprintNodeGraph | FlowGraph | StateTree |
|----------|------------------|-----------|-----------|
| **Loop Delay** | ✅ Loop Delay native node | Need to combine nodes | Not applicable (state machine model unsuitable for linear loop flow) |
| **Multi-input Sync / Parallel Branches** | ✅ Wait Branches native node | Need nested Parallel nodes, complex hierarchy | Not applicable (state machine model doesn't support native parallel) |
| **Async Loading / Conditional Wait** | ✅ Wait Condition native node | Need to build wait logic manually | Need to build event listeners manually, high implementation cost |
| **Complex Tasks / Story / Level Flow** | ✅ Native adaptation, supports resume from checkpoint / network sync, validated in production | ✅ General flow framework, general adaptability, needs additional business adaptation | ❌ Not applicable, state machine model doesn't match linear + branching task logic |
| **AI State Machine / Behavior Control** | Can implement (with Blueprint / events, not core advantage) | Can implement (needs lots of custom nodes) | ✅ Best fit, hierarchical state machine naturally adapts to AI behavior |

### 💬 One-line Summary

> **BlueprintNodeGraph = FlowGraph's ease of use + StateTree's powerful features**

- ✅ Simple scenarios: drag nodes directly, done in seconds
- ✅ No new concepts to learn
- ✅ Async support, out of the box
- ✅ Simpler than FlowGraph, more direct than StateTree

## Features

### 🚀 Core Features

- **Latent Execution Nodes** - Loop Delay, For Loop with Delay, etc.
- **Conditional Wait Nodes** - Wait Condition, Wait Branch
- **Async Delegate System** - Supports multiple input branch synchronization
- **Latent Task Framework** - Complete task lifecycle management
- **Timeout Mechanism** - Prevents infinite waiting
- **Network Replication** - Supports multiplayer games
- **GC Protection** - Automatic object lifecycle management
- **WorldPartition Support** - Large map sub-level async load/unload
- **Async Asset Loading** - Supports UObject and UClass async loading
- **Resume from Checkpoint / Save** - Complete flow state save and restore
- **Content Browser Integration** - Right-click to quickly create Flow Graph assets

### 🔧 Technical Highlights

- Developed based on latest UE5 features
- Complete C++ and Blueprint support
- Modular architecture, easy to extend
- Includes Editor integration
- Provides complete examples

### 📦 Included Modules

| Module | Type | Description |
|--------|------|-------------|
| BlueprintNodeGraph | Runtime | Runtime core module |
| BlueprintNodeGraphEditor | Editor | Editor extension module |

## Quick Start

### Installation

1. Copy the `BlueprintNodeGraph` folder to your project's `Plugins` directory
2. Reopen the project
3. Enable "BlueprintNodeGraph" in the Plugins menu

See [Installation Guide](Docs/Usage.md#installation) for detailed installation steps.

### Your First Async Node

Add a simple delay loop in Blueprint:

```
Event BeginPlay
    ↓
Loop Delay (Duration: 1.0)
    ↓ (Loop)
Print String ("Hello!")
    ↓
(Completed)
Print String ("Done!")
```

That's it! See [Quick Start Guide](Docs/Usage.md#quick-start) for more.

## Documentation

### 📚 Main Documentation

- **[Usage Guide](Docs/Usage.md)** - Complete usage guide, API reference, examples
- **[Architecture Guide](Docs/Architecture.md)** - Deep dive into plugin architecture and how it works
- **[Improvement Plan](Docs/improve.md)** - Future improvement directions and suggestions

### 🔗 Quick Links

- [Installation Guide](Docs/Usage.md#installation)
- [API Reference](Docs/Usage.md#basic-node-usage)
- [Example Project](Content/BP_TestBlueprintNodes.uasset)
- [Architecture Design](Docs/Architecture.md)

## Examples

The plugin includes multiple example Blueprints demonstrating various features:

- [BP_TestBlueprintNodes](Content/BP_TestBlueprintNodes.uasset) - Basic node usage examples
- [BP_TestTask](Content/Tasks/BP_TestTask.uasset) - Custom task example 1
- [BP_TestTask2](Content/Tasks/BP_TestTask2.uasset) - Custom task example 2

These examples are located in the `Content` folder.

## Support

### FAQ

Having issues? Check:

- [Troubleshooting](Docs/Usage.md#troubleshooting) - Solutions to common problems
- [Best Practices](Docs/Usage.md#best-practices) - Usage tips and tricks

### Reporting Issues

If you find a bug or have feature suggestions:

1. Check if an existing issue covers it
2. Provide reproduction steps and environment info
3. Use clear titles and descriptions

## Contributing

We welcome any form of contribution!

### Development Environment Setup

1. Fork this repository
2. Clone to your Unreal project's Plugins directory
3. Generate project files and compile
4. Create your feature branch

### Commit Guidelines

- Use clear commit messages
- Ensure code compiles successfully
- Add necessary documentation
- Update related examples

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## Acknowledgments

Thanks to all developers who contributed to this project!

---

### 📮 Contact Us

- Project Home: [Your Project URL]
- Issue Tracker: [Issues Page]

---

**BlueprintNodeGraph** - Making Blueprint async programming simpler!
