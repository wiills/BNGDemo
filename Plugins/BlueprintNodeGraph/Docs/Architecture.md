# BlueprintNodeGraph 架构文档

## 架构概览

```
┌─────────────────────────────────────────────────┐
│              BlueprintNodeGraph                   │
├─────────────────────────────────────────────────┤
│  ┌─────────────────┐    ┌────────────────────┐ │
│  │ Runtime Module   │    │ Editor Module      │ │
│  │ (运行时模块)      │    │ (编辑器模块)        │ │
│  └────────┬────────┘    └─────────┬──────────┘ │
│           │                      │            │
│  ┌────────▼────────┐    ┌─────────▼─────────┐  │
│  │ 核心类库          │    │ K2节点扩展        │  │
│  │ • 异步代理基类    │    │ • 自定义节点      │  │
│  │ • 延迟任务基类    │    │ • Slate UI       │  │
│  │ • 子系统管理      │    │ • 编辑器集成      │  │
│  └─────────────────┘    └───────────────────┘  │
└─────────────────────────────────────────────────┘
```

## 核心类层次

```
UObject
│
├── UExAsyncActionBase                    # 异步操作基类
│   └── UExAsyncActionProxy
│       ├── UExAsyncBlendPercentProxy     # 混合百分比
│       ├── UExWaitConditionProxy          # 条件等待
│       └── UExWaitBranchProxy             # 分支等待
│
├── UExLatentActionProxyBase              # 延迟代理基类
│   ├── UExLatentActionProxy             # 延迟代理
│   ├── UExLoopDelayProxy                 # 循环延迟
│   ├── UExForLoopWithDelayProxy          # 循环延迟
│   └── UExLatentTaskProxy               # 延迟任务代理
│
└── UExLatentTaskBase                    # 延迟任务基类
    └── UExLatentTaskForAttach            # 附加任务
```

## 模块结构

```
Source/
│
├── BlueprintNodeGraph/              # 运行时
│   ├── Public/
│   │   ├── BlueprintTool/
│   │   │   ├── ExLatentProxyDefine.h      # 核心定义
│   │   │   ├── ExAsyncActionBase.h         # 异步基类
│   │   │   ├── ExLatentActionManager.h     # 管理器
│   │   │   ├── ExLatentTaskBase.h          # 任务基类
│   │   │   └── ExLatentTaskInterface.h     # 任务接口
│   │   └── BlueprintNodeGraph.h
│   │
│   └── Private/
│       └── BlueprintTool/           # 实现
│           ├── ExAsyncActionBase.cpp
│           ├── ExLatentActionManager.cpp
│           ├── ExLatentTaskBase.cpp
│           └── ExLatentTaskInterface.cpp
│
└── BlueprintNodeGraphEditor/        # 编辑器
    ├── Public/
    │   └── BlueprintTool/
    │       ├── ExK2Node_*.h        # K2节点
    │       └── SGraphNode_*.h      # Slate节点
    │
    └── Private/
        └── BlueprintTool/
            ├── ExK2Node_*.cpp
            └── SGraphNode_*.cpp
```

## 核心流程

### 1. 代理创建流程

```
CreateWaitProxyCall<T>()
    │
    ├─→ Get UExLatentActionManager
    │       │
    │       └─→ Generate ObjectUUID
    │               │
    │               └─→ Check ProxyMap
    │                       │
    │           ┌───────────┴───────────┐
    │       存在│                       │不存在
    │           ↓                       ↓
    │       Return existing         Create NewObject
    │           │                       │
    │           │                       └─→ SetUUIDAndCount()
    │           │                               │
    │           │                               └─→ Register()
    │           │                                       │
    │           └───────────────────────────────────────┘
    │                       │
    └───────────────────────┘
                ↓
        Return Proxy
```

### 2. 节点执行流程

```
Activate()
    │
    ├─→ Decrement InputCount
    │       │
    │       └─→ Count <= 0 ?
    │               │
    │       ┌───────┴───────┐
    │      Yes             No
    │       ↓               ↓
    │   Set Finished      Wait...
    │       │
    │       └─→ OnBranchesFinished()
    │               │
    │               └─→ IsFinishAfterBranches() ?
    │                       │
    │               ┌───────┴───────┐
    │              Yes             No
    │               ↓               ↓
    │           TryFinish()      Wait...
    │               │
    └───────────────┘
```

### 3. 任务生命周期

```
┌─────────┐
│ Create  │ ── CreateTask()
└────┬────┘
     ↓
┌─────────┐
│ Pending │ ── Initial State
└────┬────┘
     ↓ Activate()
┌─────────┐
│ Running │ ── OnStart() → Execute → OnStop()
└────┬────┘
     ↓
┌─────────┐
┤ ────────┼── Completed ──→ Destroy
└─────────┘
     │
     └─→ Cancelled ──→ Destroy
```

## 关键组件

### UExLatentActionManager

```cpp
UCLASS()
class UExLatentActionManager : public UGameInstanceSubsystem
{
    // 代理对象存储
    UPROPERTY()
    TMap<FString, UObject*> ProxyMap;
    
    // 管理方法
    void SetProxyObject(const FString& Key, UObject* Proxy);
    void RemoveProxyObject(const FString& Key);
    
    template<class T>
    T* GetProxyObject(const FString& Key);
};
```

### IExLatentTaskInterface

```cpp
UENUM(BlueprintType)
enum class EExLatentTaskState : uint8
{
    Pending,
    Running,
    Completed,
    Failed,
    Cancelled
};

class IExLatentTaskInterface
{
    // 状态管理
    virtual EExLatentTaskState GetState() const = 0;
    virtual void SetState(EExLatentTaskState InState) = 0;
    
    // 生命周期
    virtual void TryStart();
    virtual void TryStop();
    virtual void Terminate();
};
```

### FExLatentNodeInfo

```cpp
USTRUCT(BlueprintType)
struct FExLatentNodeInfo
{
    UPROPERTY()
    FString UUID;              // 资源UUID
    
    UPROPERTY()
    FString UniqueId;          // 唯一ID
    
    UPROPERTY()
    FString StartLog;          // 开始日志
    
    UPROPERTY()
    FString EndLog;            // 结束日志
    
    UPROPERTY()
    float TimeOut = 0.f;       // 超时时间
};
```

## 编辑器节点扩展

```
UK2Node_ShowBase (基础节点类)
│
├── UK2Node_AsyncBase
│   ├── UK2Node_AsyncBlendPercent
│   └── UK2Node_CreateTaskAsync
│
├── UK2Node_LatentTaskCall
│   └── UK2Node_LatentTaskObject
│
├── UK2Node_LoopDelay
│   └── UK2Node_ForLoopWithDelay
│
├── UK2Node_WaitCondition
│   └── UK2Node_WaitBranch
│
└── UK2Node_SwitchValue
```

## 内存管理

### GC 防护机制

```
┌─────────────────────────────────┐
│      RF_StrongRefOnFrame        │
│  (帧强引用，防止GC回收)           │
└─────────────────────────────────┘
                ↓
┌─────────────────────────────────┐
│  RegisterWithGameInstance()      │
│  (注册到GameInstance)            │
└─────────────────────────────────┘
                ↓
┌─────────────────────────────────┐
│      TWeakObjectPtr<>            │
│  (弱引用存储，避免循环引用)        │
└─────────────────────────────────┘
                ↓
┌─────────────────────────────────┐
│     SetReadyToDestroy()          │
│  (标记可销毁)                      │
└─────────────────────────────────┘
```

### 对象生命周期

```cpp
// 1. 创建时设置强引用
UExLatentActionProxyBase(const FObjectInitializer& OI)
{
    SetFlags(RF_StrongRefOnFrame);
}

// 2. 注册到子系统
RegisterWithGameInstance(WorldContext);

// 3. 完成时释放引用
SetReadyToDestroy();

// 4. 子系统自动清理
RemoveProxyObject(UUID);
```

## 超时机制

```
SetK2NodeInfo()
    │
    ├─→ Check TimeOut > 0
    │       │
    │       └─→ SetTimer()
    │               │
    │               └─→ Lambda Callback
    │                       │
    │                       ├─→ IsValid(Object)
    │                       │       │
    │                       │       └─→ TryFinish()
    │                       │
    │                       └─→ Log Timeout
```

## 网络复制

```
Server                              Client
  │                                   │
  ├─→ CreateTask()                   │
  │       │                           │
  │       ├─→ SetReplicates(true)     │
  │       │                           │
  │       └─→ Replicate State         │
  │               │                   │
  │               └─→ OnRep_State ────┤
  │                                   │
  └── TryStop()                       │
          │                           │
          └─→ Replicate Stop ─────────┘
```

## 扩展指南

### 扩展异步代理

```cpp
UCLASS()
class UMyAsyncProxy : public UExLatentActionProxyBase
{
    GENERATED_BODY()
    
public:
    // 重写分支完成回调
    virtual void OnBranchesFinished() override;
    
    // 重写完成回调
    virtual void OnFinishCall() override;
    
    // 重写是否自动完成
    virtual bool IsFinishAfterBranches() const override { return true; }
};
```

### 扩展延迟任务

```cpp
UCLASS(Blueprintable)
class UMyLatentTask : public UExLatentTaskBase
{
    GENERATED_BODY()
    
public:
    // C++ 实现
    virtual void OnStart() override;
    virtual void OnStop() override;
    
    // 蓝图实现
    UFUNCTION(BlueprintImplementableEvent)
    void ReceiveOnStart();
    
    UFUNCTION(BlueprintImplementableEvent)
    void ReceiveOnStop();
};
```

### 扩展 K2 节点

```cpp
class UMyK2Node : public UK2Node_ShowBase
{
public:
    // 创建引脚
    virtual void AllocateDefaultPins() override;
    
    // 节点展开
    virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* OutputGraph) override;
    
    // 菜单分类
    virtual FText GetMenuCategory() const override;
    
    // 节点标题
    virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
};
```

## 数据流图

```
输入流程：
Input A ──┐
Input B ──┼──→ Wait All ──→ Output
Input C ──┘

条件流程：
Condition ──→ Is True? ──→ Yes ──→ Continue
                    │
                    No
                    ↓
               Wait...

循环流程：
Start ──→ Loop ──→ Body ──→ Delay ──→ Loop?
                                   │
                         ┌─────────┴─────────┐
                        Yes                  No
                         ↓                    ↓
                      Continue            Completed
```
