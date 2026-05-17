# BlueprintNodeGraph 使用文档

## 快速开始

### 安装

```
项目/Plugins/ → 复制 BlueprintNodeGraph 文件夹 → 重启引擎 → 启用插件
```

### 最简单的例子

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

## 节点速查

### 1. Loop Delay - 循环延迟

```
参数：
  Duration: Float      - 延迟时间（秒）
  Count: Integer      - 循环次数
  Need First Delay: Bool - 首次是否延迟

引脚：
  → Exec       - 执行输入
  → Loop       - 每次循环输出
  → Completed  - 全部完成输出
```

**示例：闪烁效果**

```
Event BeginPlay
    ↓
Loop Delay (Duration: 0.5, Count: 5)
    ├─ Loop → Toggle Visibility (Light)
    └─ Completed → Print String ("Done!")
```

---

### 2. For Loop with Delay - 带延迟的 For 循环

```
参数：
  Loop Count: Integer     - 总次数
  Loop Interval: Float   - 间隔时间
  Need First Delay: Bool - 首次延迟

引脚：
  → Loop Body (Index) - 带索引的循环体
  → Completed         - 完成
```

**示例：逐个生成物体**

```
For Loop with Delay (Count: 10, Interval: 1.0)
    Loop Body (Index) → Spawn Actor (Location: Index * 100)
    Completed → Print String ("All Spawned!")
```

---

### 3. Wait Condition - 等待条件

```
参数：
  Condition: Bool& - 布尔引用

引脚：
  多个 → Exec     - 多输入分支
  → Completed     - 条件满足
```

**示例：等待玩家进入区域**

```
Overlap Event
    ↓
Set bPlayerReady = true
    ↓
Wait Condition (bPlayerReady)
    ↓
Open Door
```

---

### 4. Async Blend Percent - 异步混合

```
参数：
  Percent Speed 1: Float& - 速度引用1
  Percent Speed 2: Float& - 速度引用2

引脚：
  多个 → Exec       - 输入分支
  → Completed       - 完成
```

---

## 延迟任务系统

### 概述

延迟任务是一种可复用、可序列化的异步执行单元，适合复杂的多步骤逻辑。

```
任务优势：
✓ 可在蓝图/C++ 中使用
✓ 支持网络复制
✓ 自动 GC 防护
✓ 可中断/恢复
✓ 完整的生命周期管理
```

---

### 蓝图自定义任务节点（完整指南）

#### 步骤 1：创建任务蓝图类

```
1. 打开内容浏览器
2. 右键 → Blueprint Class
3. 搜索 "ExLatentTaskProxy"
4. 选择并命名（如 BP_LevelTask）
```

> ⚠️ 注意：使用 `ExLatentTaskProxy` 而不是 `ExLatentTaskBase`，因为 K2 节点调用的是 `ExLatentTaskProxy`

#### 步骤 2：配置任务属性

```
BP_LevelTask → Class Settings → Details:

├─ Description: "我的自定义任务"
├─ Parent Class: ExLatentTaskProxy
└─ 保持默认即可
```

#### 步骤 3：实现事件回调

在事件图表中添加：

```
┌──────────────────────────────────────────────────────┐
│  BP_LevelTask 事件图表                                │
├──────────────────────────────────────────────────────┤
│                                                       │
│  Receive On Start (事件)                              │
│      ↓                                               │
│  [在此实现任务开始逻辑]                                 │
│      ↓                                               │
│  Call On Task Finished (调用函数)                      │
│                                                       │
├──────────────────────────────────────────────────────┤
│                                                       │
│  Receive On Stop (事件)                               │
│      ↓                                               │
│  [在此实现清理逻辑，如取消定时器]                         │
│                                                       │
└──────────────────────────────────────────────────────┘
```

#### 步骤 4：添加任务参数（可选）

```
如需传递参数，在事件中接收：

1. 添加变量（如 TargetActor: Actor）
2. 在事件中读取：
   
   Receive On Start
       ↓
   Get TargetActor
       ↓
   调用任务逻辑...
```

#### 步骤 5：在其他蓝图调用任务

```
主蓝图：

1. 拖入 "Create Latent Task" 节点
2. 选择 BP_LevelTask
3. 设置参数（TargetActor 等）
4. 绑定委托
5. 调用 Activate

完整流程：
Event BeginPlay
    ↓
Create Latent Task (BP_LevelTask, TargetActor: Self)
    ├─ On Start → Print String ("任务开始")
    └─ On Complete → Print String ("任务完成")
    ↓
Activate
```

---

### 常用任务模式

#### 模式 1：定时任务

```
BP_TimerTask:

Receive On Start
    ↓
Set Timer by Event (Duration: 2.0, Loop: No)
    ↓
Timer Event
    ↓
Call On Task Finished
```

#### 模式 2：监听任务

```
BP_ListenerTask:

Receive On Start
    ↓
Bind Event to OnOverlap (BoxCollision)
    ↓
OnOverlap Event
    ↓
Call On Task Finished
```

#### 模式 3：序列任务

```
BP_SequenceTask:

Receive On Start
    ├─ Action 1 → Delay (0.5s)
    ├─ Action 2 → Delay (0.5s)
    ├─ Action 3 → Delay (0.5s)
    └─ Call On Task Finished
```

#### 模式 4：循环任务

```
BP_LoopTask:

变量: LoopCount = 0

Receive On Start
    ├─ Set LoopCount = 0
    └─ Start Loop

Loop Body (自定义事件)
    ├─ LoopCount + 1
    ├─ 执行任务逻辑
    └─ LoopCount >= 10?
            ├─ Yes → Call On Task Finished
            └─ No → Set Timer (1.0s) → Loop Body
```

---

### 完整示例：武器攻击任务

```
BP_WeaponAttackTask (武器攻击延迟任务):

变量:
├─ WeaponActor: Actor
├─ Damage: Float = 50.0
└─ TargetActor: Actor

事件图表:

Receive On Start
    ├─ Get TargetActor
    ├─ Apply Damage (TargetActor, Damage)
    ├─ Play Animation (WeaponActor, AttackAnim)
    ├─ Delay (1.0s)
    ├─ Play Sound (ImpactSound)
    └─ Call On Task Finished

Receive On Stop
    ├─ Stop Animation (WeaponActor)
    └─ 清理资源...
```

**使用方式：**

```
Character 蓝图:

Event Attack
    ↓
Create Latent Task (BP_WeaponAttackTask)
    ├─ Set WeaponActor = ThisWeapon
    ├─ Set TargetActor = EnemyTarget
    ├─ Set Damage = 50.0
    ├─ On Start → Play Attack Montage
    └─ On Complete → Reset Attack State
    ↓
Activate
```

---

### 任务生命周期

```
CreateTask() → Activate() → OnStart() → [执行中] → OnStop() → 销毁

状态流转：
  Pending → Running → Completed
              ↓
          Cancelled
```

## C++ API

### 创建任务

```cpp
// 创建
UExLatentTaskBase* Task = UExLatentTaskBase::CreateTask(World, TaskClass);

// 配置
Task->SetK2NodeInfo(NodeInfo);

// 绑定
Task->CompleteDelegate.AddDynamic(this, &UMyClass::OnComplete);

// 启动
Task->Activate();
```

### 状态管理

```cpp
// 查询
EExLatentTaskState State = Task->GetState();

// 控制
Task->TryStart();   // 开始
Task->TryStop();    // 停止
Task->Terminate();  // 终止
```

### 枚举定义

```cpp
enum class EExLatentTaskState : uint8
{
    Pending,    // 等待
    Running,    // 执行中
    Completed,  // 完成
    Failed,     // 失败
    Cancelled   // 取消
};
```

## 高级配置

### 超时设置

```
节点属性面板 → Node Info:
    ├─ Time Out: 10.0        ← 超时时间（秒）
    ├─ Start Log: "开始"     ← 开始日志
    └─ End Log: "结束"       ← 结束日志
```

### GC 防护

```cpp
// 注册（防止被回收）
Task->RegisterWithGameInstance(WorldContext);

// 释放
Task->SetReadyToDestroy();
```

## 流程图库

### 并行加载

```
          ┌─ Load Asset A ─┐
Load All ─┼─ Load Asset B ─┼─ All Loaded ─→ 继续
          └─ Load Asset C ─┘
```

### 条件触发

```
         ┌─ 条件A满足 ─┐
Any Of ─┼─ 条件B满足 ─┼─ 任一完成
         └─ 条件C满足 ─┘
```

### 状态机模式

```
Pending ──→ [激活] ──→ Running ──→ [完成] ──→ Completed
                    ↓
              [取消] ──→ Cancelled
```

## 调试

### 日志输出

```
Output Log 中查看:
[StartLog] - MyNode, 开始执行
[EndLog]   - MyNode, 执行完成
```

### 调试技巧

```
✓ 设置 Start/End Log 追踪流程
✓ 检查 World Context 是否正确
✓ 使用 IsValid() 检查对象
✓ 监控 ProxyMap 中的对象数量
```

## 常见问题

| 问题 | 解决方案 |
|------|----------|
| 节点不执行 | 检查 World Context 连接 |
| 崩溃 | 使用 IsValid() 检查指针 |
| 不同步 | 确保 bReplicates = true |
| GC回收 | 调用 RegisterWithGameInstance() |

## 示例文件

```
Content/
├─ BP_TestBlueprintNodes.uasset  ← 节点示例
└─ Tasks/
   ├─ BP_TestTask.uasset        ← 任务示例1
   └─ BP_TestTask2.uasset       ← 任务示例2
```
