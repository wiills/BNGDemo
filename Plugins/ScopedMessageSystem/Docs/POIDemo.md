# Scoped Message Poi 示例

这个示例是插件的手动测试样例，用来验证两个相同模板的 Poi 实例可以使用同一组消息
Channel，但彼此之间不会串消息。

## Actor

- `AScopedMessagePoiDemoRoot`：Poi 根节点，带有 `UScopedMessageScopeComponent`。
  默认会在服务端 BeginPlay 时自动注册当前 PlayerController，方便测试
  `ServerToScopedClients`。它继承自通用基类 `AScopedMessagePoiRootActor`。
- `AScopedMessagePoiDemoTerminal`：广播 `Poi.Demo.Terminal.Activated`。
- `AScopedMessagePoiDemoDoor`：监听同一个 Scope 内的终端激活消息。
  Terminal 和 Door 都继承自 `AScopedMessagePoiSubActor`，会等待本地能解析出有效
  `ScopeId` 后再执行自己的 Poi 逻辑。

## 搭建方式

1. 在关卡里放置两个 `AScopedMessagePoiDemoRoot`。
2. 在每个 Root 下放置或 Attach 一个 `AScopedMessagePoiDemoTerminal`。
3. 在每个 Root 下放置或 Attach 一个 `AScopedMessagePoiDemoDoor`。
4. 两个 Terminal 都使用 Channel `Poi.Demo.Terminal.Activated`。
5. 两个 Door 都监听 Channel `Poi.Demo.Terminal.Activated`。
6. 将每个 Door 的 `RequiredTerminalId` 设置为同一个 Poi 内 Terminal 的 ID。

关键点是 Attach 或 Owner 关系：Terminal 和 Door 必须能通过 Scope Resolver 解析到同一个
Poi Root。默认 Resolver 支持直接 Provider、Actor Component、Owner 链、Attach 父级链和
Outer 链。如果项目里的 Poi 归属关系存放在其他系统里，推荐注册自定义 Scope Resolver，
而不是修改这些 Demo Actor。

## 网络设置

如果只是单进程编辑器验证，可以让 Terminal 和 Door 使用 `LocalOnly`。这样可以先验证本地
Scope 隔离，不需要注册玩家兴趣范围。

如果使用 `ServerToScopedClients`，需要在服务端注册对该 Scope 感兴趣的玩家：

```cpp
DemoRoot->RegisterPlayer(PlayerController);
```

Demo Root 默认开启 `bAutoRegisterPlayersOnBeginPlay`，会在服务端等待 `ScopeId` 有效且
当前世界里已有 PlayerController 后，把当前 PlayerController 注册到自己的 Scope，方便在
PIE 里直接验证。如果你想手动控制玩家兴趣范围，可以关闭这个开关，然后按项目逻辑调用
`RegisterPlayer` 和 `UnregisterPlayer`。

真实 Poi 系统里，玩家进入、激活、或流式加载到 Poi 时调用注册；玩家离开 Poi 时调用
`UnregisterPlayer`。`ActivateTerminal` 是服务端权威逻辑：客户端调用会先转发到服务端，
再由服务端判断 Scope 并投递消息。

## 测试

调用 Poi A 中 Terminal 的 `ActivateTerminal`。

预期结果：

- Poi A 里的 Door 打开。
- Poi B 里的 Door 保持关闭。
- 日志显示两个 Poi 使用相同 Channel，但 `ScopeId` 不同。

调用 Poi B 中 Terminal 的 `ActivateTerminal`。

预期结果：

- Poi B 里的 Door 打开。
- Poi A 里的 Door 不受影响。
