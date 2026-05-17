# BlueprintNodeGraph
## UE5 轻量级并行流程编排框架（蓝图原生）
### Production-Ready · Battle-tested in Shipped Games

BlueprintNodeGraph 是专为UE5设计的**蓝图原生异步流程框架**，替代零散延迟/等待节点、简化任务/剧情/关卡逻辑开发，**支撑上线项目全关卡逻辑验证**。

核心优势：**标准蓝图内直接使用、原生并行异步、GC安全、网络同步、低开销易扩展**。

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Unreal Engine](https://img.shields.io/badge/unreal--engine-5.7+-green.svg)](https://www.unrealengine.com/)
[![Version](https://img.shields.io/badge/version-1.1-orange.svg)](https://github.com/your-repo)

## 目录

- [项目简介](#项目简介)
- [为什么选择 BlueprintNodeGraph？](#为什么选择-blueprintnodegraph)
- [功能特性](#功能特性)
- [快速开始](#快速开始)
- [文档](#文档)
- [示例](#示例)
- [技术支持](#技术支持)
- [贡献指南](#贡献指南)
- [许可证](#许可证)

## 项目简介

BlueprintNodeGraph 是一个强大的 Unreal Engine 插件，提供了丰富的异步蓝图节点和延迟任务执行系统。该插件让开发者能够在蓝图中轻松实现复杂的异步逻辑，无需编写大量 C++ 代码。

**主要设计目标：**
- 提供直观、易用的异步节点
- 完整的任务生命周期管理
- GC 安全的对象管理
- 网络复制支持
- 高性能、低开销

**核心文件：**
- [ExLatentTaskBase.h](Source/BlueprintNodeGraph/Public/BlueprintTool/ExLatentTaskBase.h) - 延迟任务基类
- [ExLatentActionManager.h](Source/BlueprintNodeGraph/Public/BlueprintTool/ExLatentActionManager.h) - 代理管理器
- [ExK2Node_LatentTaskCall.h](Source/BlueprintNodeGraphEditor/Public/BlueprintTool/ExK2Node_LatentTaskCall.h) - K2节点扩展

## 为什么选择 BlueprintNodeGraph？

### 🆚 与其他方案对比

| 功能 | **BlueprintNodeGraph** | FlowGraph | StateTree |
|------|---------------------|-----------|-----------|
| **核心定位** | 纯蓝图异步节点系统 | 可视化节点编程 | 状态机系统 |
| **学习曲线** | ✅ **极低**（像用现成节点一样**） | 高（需学习特定概念） | 高（需学习状态机概念） |
| **使用方式** | ✅ **直接在标准蓝图编辑器中拖节点** | 需要打开单独的 FlowGraph 编辑器 | 需要打开专门的编辑器 |
| **异步执行** | ✅ **原生异步支持** | 需要额外实现 | 主要同步 |
| **超时机制** | ✅ **内置超时保护** | 需要自行实现 | 需要自行实现 |
| **GC 防护** | ✅ **自动GC安全** | 需要手动管理 | 需要手动管理 |
| **网络复制** | ✅ **直接支持** | 需要自行实现 | 需要自行实现 |
| **延迟循环** | ✅ **Loop Delay 等现成节点** | 需要组合节点实现 | 不适用 |
| **多分支同步** | ✅ **Wait Branches 等现成节点** | 需要手动搭建 | 需要组合状态切换 |

### 💡 我们的核心优势

#### 1. 直接在标准蓝图编辑器中工作

**其他方案：**
- FlowGraph 需要使用单独的编辑器
- StateTree 需要专门的编辑器

**我们的方案：**
- ✅ 直接在标准蓝图编辑器中拖入节点
- ✅ 无需学习新的编辑器
- ✅ 与你的现有工作流完全兼容
- ✅ 学习成本几乎为零

#### 2. 开箱即用的异步执行

**文件位置：** [ExLatentProxyDefine.h](Source/BlueprintNodeGraph/Public/BlueprintTool/ExLatentProxyDefine.h)

```cpp
// 直接拖入节点即用：
Loop Delay (Duration: 1.0)
    ↓
执行你的逻辑
```

无需配置复杂的图表，直接使用！

#### 3. 自动 GC 防护机制

**文件位置：** [ExAsyncActionBase.h#L96-113](Source/BlueprintNodeGraph/Public/BlueprintTool/ExAsyncActionBase.h#L96-113)

```cpp
// 自动处理对象生命周期：
RegisterWithGameInstance(WorldContext);  // 防止被回收
SetReadyToDestroy();                    // 完成后标记销毁
```

- ✅ 自动管理，无需手动清理
- ✅ 无内存泄漏风险

#### 4. 超时机制保护

**文件位置：** [ExLatentNodeInfo](Source/BlueprintNodeGraph/Public/BlueprintTool/ExLatentProxyDefine.h)

```
节点属性 → 配置 Time Out: 10.0
    ↓
自动超时保护
```

防止无限等待！

#### 5. 完整任务生命周期

**文件位置：** [ExLatentTaskBase.h#L61](Source/BlueprintNodeGraph/Public/BlueprintTool/ExLatentTaskBase.h#L61)

```
CreateTask() → Activate() → Receive On Start → Receive On Stop → Destroy
```

- 完整的状态机管理
- 支持网络复制同步

### 🎯 使用场景对比

| 场景  | BlueprintNodeGraph | FlowGraph | StateTree |
| --- | --- | --- | --- |
| **循环延迟** | ✅ Loop Delay 原生节点 | 需要组合节点实现 | 不适用（状态机模型不适合线性循环流程） |
| **多输入同步 / 并行分支** | ✅ Wait Branches 原生节点 | 需要嵌套 Parallel 节点，层级复杂 | 不适用（状态机模型不支持原生并行） |
| **异步加载 / 条件等待** | ✅ Wait Condition 原生节点 | 需要手动搭建等待逻辑 | 需要手动搭建事件监听，实现成本高 |
| **复杂任务 / 剧情 / 关卡流程** | ✅ 原生适配，支持断点续跑 / 网络同步，上线验证 | ✅ 通用流程框架，适配性一般，需额外适配业务 | ❌ 不适用，状态机模型不匹配任务线性 + 分支逻辑 |
| **AI 状态机 / 行为控制** | 可实现（配合蓝图 / 事件，非核心优势） | 可实现（需大量自定义节点） | ✅ 最适合，分层状态机天生适配 AI 行为 |

### 💬 一句话总结

> **BlueprintNodeGraph = FlowGraph 的易用性 + StateTree 的强大功能**

- ✅ 简单场景：直接拖入节点，几秒钟搞定
- ✅ 无需学习新概念
- ✅ 支持异步，开箱即用
- ✅ 比 FlowGraph 更简单，比 StateTree 更直接

## 功能特性

### 🚀 核心功能

- **延迟执行节点** - Loop Delay、For Loop with Delay 等
- **条件等待节点** - Wait Condition、Wait Branch
- **异步代理系统** - 支持多个输入分支同步
- **延迟任务框架** - 完整的任务生命周期管理
- **超时机制** - 防止无限等待
- **网络复制** - 支持多人游戏
- **GC 防护** - 自动管理对象生命周期
- **WorldPartition 支持** - 大地图子关卡异步加载/卸载
- **异步资产加载** - 支持 UObject 和 UClass 异步加载
- **断点续跑/存档** - 完整的流程状态保存和恢复
- **内容浏览器集成** - 右键快速创建 Flow Graph 资产

### 🔧 技术亮点

- 基于 UE5 最新特性开发
- 完整的 C++ 和蓝图支持
- 模块化架构，易于扩展
- 包含编辑器集成
- 提供完整示例

### 📦 包含模块

| 模块 | 类型 | 描述 |
|------|------|------|
| BlueprintNodeGraph | Runtime | 运行时核心模块 |
| BlueprintNodeGraphEditor | Editor | 编辑器扩展模块 |

## 快速开始

### 安装

1. 将 `BlueprintNodeGraph` 文件夹复制到项目的 `Plugins` 目录
2. 重新打开项目
3. 在 Plugins 菜单中启用 "BlueprintNodeGraph"

详细安装步骤请查看 [使用文档](Docs/Usage.md#安装指南)。

### 你的第一个异步节点

在蓝图中添加一个简单的延迟循环：

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

就这么简单！查看 [快速开始指南](Docs/Usage.md#快速开始) 了解更多。

## 文档

### 📚 主要文档

- **[使用文档](Docs/Usage.md)** - 完整的使用指南、API 参考、示例
- **[架构文档](Docs/Architecture.md)** - 深入了解插件架构、工作原理
- **[改进计划](Docs/improve.md)** - 未来的改进方向和建议

### 🔗 快速链接

- [安装指南](Docs/Usage.md#安装指南)
- [API 参考](Docs/Usage.md#基础节点使用)
- [示例项目](Content/BP_TestBlueprintNodes.uasset)
- [架构设计](Docs/Architecture.md)

## 示例

插件包含多个示例蓝图，展示各种功能：

- [BP_TestBlueprintNodes](Content/BP_TestBlueprintNodes.uasset) - 基础节点使用示例
- [BP_TestTask](Content/Tasks/BP_TestTask.uasset) - 自定义任务示例 1
- [BP_TestTask2](Content/Tasks/BP_TestTask2.uasset) - 自定义任务示例 2

这些示例位于 `Content` 文件夹中。

## 技术支持

### 常见问题

遇到问题？请查看：

- [故障排除](Docs/Usage.md#故障排除) - 常见问题的解决方案
- [最佳实践](Docs/Usage.md#最佳实践) - 使用建议和技巧

### 报告问题

如果发现 bug 或有功能建议：

1. 检查是否已有相关 issue
2. 提供复现步骤和环境信息
3. 使用清晰的标题和描述

## 贡献指南

我们欢迎任何形式的贡献！

### 开发环境设置

1. Fork 本仓库
2. 克隆到你的 Unreal 项目 Plugins 目录
3. 生成项目文件并编译
4. 创建你的特性分支

### 提交规范

- 使用清晰的 commit message
- 确保代码编译通过
- 添加必要的文档
- 更新相关示例

## 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件。

---

## 致谢

感谢所有为这个项目做出贡献的开发者！

---

### 📮 联系我们

- 项目主页: [你的项目地址]
- 问题反馈: [Issues 页面]

---

**BlueprintNodeGraph** - 让蓝图异步编程更简单！
