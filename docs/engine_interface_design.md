# UE5 启发式引擎接口设计

## 文档定位

本文替代之前的 `EntityId / RenderWorldSnapshot` 主线接口设计。当前接口设计以 UE5 的框架层次为参考，但保持适合本项目规模的最小实现。

## 核心层次

```text
Engine
  EngineSubsystem
  World
    Level
      Actor
        ActorComponent
          SceneComponent
```

## EngineObject

所有框架对象的最小公共基类。

职责：

- 提供稳定运行时对象 ID。
- 提供对象名称。
- 不承担序列化、反射、GC。

当前接口：

```cpp
using ObjectId = std::uint64_t;

class EngineObject
{
public:
    virtual ~EngineObject() = default;

    ObjectId getObjectId() const;
    const std::string& getName() const;
    void setName(std::string name);
};
```

## Engine

职责：

- 持有 `EngineContext`。
- 管理 Engine 生命周期。
- 持有当前 active `World`。
- 持有 `EngineSubsystem`。

当前接口：

```cpp
class Engine
{
public:
    bool initialize(const EngineDesc& desc);
    void tick(float deltaSeconds);
    void shutdown();

    World& createWorld(std::string name);
    World* getActiveWorld() const;
};
```

## World

职责：

- 表示一个正在运行或编辑的世界。
- 持有 persistent level。
- 分发 beginPlay / tick / endPlay。

当前接口：

```cpp
class World : public EngineObject
{
public:
    Level& createPersistentLevel(std::string name);
    Level* getPersistentLevel() const;

    void beginPlay();
    void tick(float deltaSeconds);
    void endPlay();
};
```

## Level

职责：

- 组织 Actor。
- 作为未来地图分块、streaming level、场景保存的基础。

当前接口：

```cpp
class Level : public EngineObject
{
public:
    template <class T, class... Args>
    T& spawnActor(Args&&... args);

    void beginPlay();
    void tick(float deltaSeconds);
    void endPlay();
};
```

## Actor

职责：

- 表示可放入 World / Level 的运行时对象。
- 持有 Component。
- 提供 beginPlay / tick / endPlay。

当前接口：

```cpp
class Actor : public EngineObject
{
public:
    template <class T, class... Args>
    T& createComponent(Args&&... args);

    SceneComponent* getRootComponent() const;
    void setRootComponent(SceneComponent* rootComponent);

    virtual void beginPlay();
    virtual void tick(float deltaSeconds);
    virtual void endPlay();
};
```

## ActorComponent

职责：

- 表示 Actor 的可组合能力。
- 支持 active / tick 开关。
- 后续 Renderer、Camera、Light、Mesh 都可以迁移成组件。

当前接口：

```cpp
class ActorComponent : public EngineObject
{
public:
    Actor* getOwner() const;
    bool isActive() const;
    void setActive(bool active);
    bool canTick() const;
    void setCanTick(bool canTick);

    virtual void onRegister();
    virtual void beginPlay();
    virtual void tick(float deltaSeconds);
    virtual void endPlay();
};
```

## SceneComponent

职责：

- 表示带空间变换和层级关系的组件。
- 对应 UE 中 `USceneComponent` 的最小版本。

当前接口：

```cpp
class SceneComponent : public ActorComponent
{
public:
    const Transform& getRelativeTransform() const;
    void setRelativeTransform(const Transform& transform);

    SceneComponent* getParent() const;
    const std::vector<SceneComponent*>& getChildren() const;
    void attachTo(SceneComponent* parent);
    void detachFromParent();
};
```

## EngineSubsystem

职责：

- 表示跨 World 的 Engine 级服务。
- 后续 `RendererSubsystem`、`AssetSubsystem`、`EditorSubsystem` 都应走这条路。

当前接口：

```cpp
class EngineSubsystem
{
public:
    virtual bool initialize(EngineContext& context) = 0;
    virtual void tick(EngineContext& context);
    virtual void shutdown(EngineContext& context) = 0;
};
```

## 后续接口迁移顺序

1. `RendererSubsystem` 包装现有 renderer。已完成第一版非侵入式 bridge，并已进入 Engine-owned frame bridge。
2. `MeshComponent` 持有旧 `Mesh` 或 mesh adapter。已完成第一版。
3. `LightComponent` 持有旧 `Light` 或 light adapter。已完成第一版。
4. `CameraComponent` 持有旧 `Camera` 或 camera adapter。已完成第一版，当前为 non-owning pointer。
5. `LegacyObjectComponent` / `LegacyObjectActor` 持有旧 `Object`。已完成第一版。
6. `LegacySceneWorldBuilder` 把旧 `Scene / Object` 树递归导入 `World / Level / Actor`。已完成第一版非侵入式 bridge。
7. `WorldLegacySceneExporter` 把 `World / Level / Actor` 导出为旧 renderer 所需 `Scene`。已完成第一版非侵入式 bridge。
8. `--verify-engine-world-scene-probe` 证明受控 probe 可以先生成 World，再回填旧 Scene，并通过 deferred PBR 渲染。
9. `WorldDrivenSceneSetup` 把 verification-only probe 提取成 `tools/sceneSetup` 下可复用 World-driven scene setup helper。已完成第一版。
10. `SceneSetupPipeline` 把 runtime scene prepare 改为第一层可配置入口，默认仍调用旧 `prepareDefaultScene(...)`。已完成第一版。
11. `--verify-engine-world-scene-probe` 已通过 `SceneSetupPipelineConfig` 接入 scene prepare 阶段，不再由 PBR verification 后置添加。
12. `SceneSetup` 已拆出 `prepareSceneInfrastructure(...)`、`prepareLegacyDefaultSceneContent(...)` 和 `prepareConfiguredPBRPreview(...)`，让 pipeline 可以只准备基础渲染设施。
13. `--verify-engine-world-minimal-scene` 已通过，当前最小 World-driven scene 由 root `LegacyObjectActor` 和 4 个 child `MeshActor` 组成，导出后保持 1 个 scene root 和 4 条 object attachment。
14. `AppRuntimeContext::engineWorld` 已作为 runtime 持久 World 状态源接入，World-driven scene setup 会填充该 World 后再导出给旧 renderer。
15. `RuntimePBRVerification` 已输出 `runtimeWorldActors`，验证 `engine-world-scene-probe` 为 2 个 Actor、`engine-world-minimal-scene` 为 5 个 Actor。
16. `engineWorld` 已暴露给 Editor hierarchy，旧 Object tree 和新 World / Level / Actor tree 当前可以并行观察。
17. Editor selection 已支持 Actor，Actor inspector 已只读展示 Actor 基础信息、所属 World / Level、组件列表和 root `SceneComponent` relative transform。
18. 默认旧 scene path 已生成 read-only World mirror，减少了只有 World-driven verification mode 才有 `engineWorld` 的割裂。
19. Actor / Component inspector 已接入 `PropertyBuilder / drawProperties(...)` schema，当前统一描述只读 Actor 字段、Component 字段、SceneComponent transform 和 adapter 绑定信息。
20. Actor / Component edit commit bridge 已完成第一版：World-driven scene 中具备旧 `Object` adapter 的 `SceneComponent` transform 编辑可以同步 adapter 持有的旧 `Object` transform；legacy mirror 继续只读。
21. Component direct selection 已完成第一版：hierarchy 中 Component 节点进入 `SelectionKind::Component`，inspector 使用专用 Component schema。
22. 最小 edit transaction boundary 已完成第一版：`SceneComponent` transform 编辑记录 target、field、before、after，并在 inspector 展示 dirty / latest record；当前尚未实现 undo/redo。
23. 最小 undo apply 已完成第一版：可以撤销最近一条 `SceneComponent` Vec3 transform transaction，并同步 `SceneComponent` 与 adapter 旧对象；当前尚未实现 redo stack。
24. Dirty/save transaction boundary 已完成第一版：`EditTransactionLog` 提供 `markDirty()`、`markSaved()` 和 `clear()`，inspector 提供 `Mark Saved / Clear Transactions` 入口。
25. Scene transform snapshot persistence boundary 已完成第一版：`saveSceneTransformSnapshot(...)` 可保存 World-driven scene 中的 `SceneComponent` relative transform。
26. Scene transform snapshot load/apply boundary 已完成第一版：`applySceneTransformSnapshot(...)` 可读取 snapshot 并回放 `SceneComponent` relative transform，同时同步旧 adapter 对象。
27. Stable scene path id 已完成第一版：snapshot 写入 actor/component stable path，apply 优先按 stable path 匹配，再 fallback 到 object id 与 index。
28. Persistent id 字段已完成第一版：`EngineObject` 持有可序列化 persistent id，World-driven preset 为 `World / Level / Actor / SceneComponent` 写入确定性 id，snapshot apply 优先按 persistent id 匹配。
29. Legacy mirror persistent id coverage 已完成第一版：旧 `Scene / Object` 树导入时会为 mirror World、Level、Actor 和 root SceneComponent 派生 deterministic persistent id。
30. Persistent id policy module 已完成第一版：`engine/PersistentIdPolicy` 统一 id 格式为 `objectKind:source:scope:path`，并显式定义 preset、legacy mirror、imported asset 和 editor-created 来源。
31. Imported asset persistent id coverage 已完成第一版：Assimp PBR import probe 导入 runtime `engineWorld` 时会使用 `ImportedAssetDerived` 来源生成 Actor / SceneComponent persistent id。
32. Editor-created persistent id coverage 已完成第一版：`EditorWorldActions::createEditorEmptyActor(...)` 会创建普通 Actor + root SceneComponent，并分配 `EditorCreatedGenerated` 来源 id。
33. Scene package save/load boundary 已完成第一版：`ScenePackage` 保存 `World / Level / Actor / SceneComponent` name、persistent id、root component、parent component 和 transform，并能加载成新的 runtime `World`。
34. Scene package component type / adapter metadata 已完成第一版：package 写出 actor/component type、adapter kind、runtime asset reference、object type 和 material type，load 阶段恢复 typed component 类。
35. Scene package asset resolver / renderer reconstruction 已完成第一版：`ScenePackageAssetResolver` 作为加载阶段的外部资源解析接口，允许 package-loaded typed component 回填真实 mesh / light / legacy renderer object。
36. package-loaded World export verification 已完成第一版：`engine-world-scene-package` mode 会把加载后的 World 重新导出到旧 renderer scene，并断言 mesh / light / legacy object / attachment 数量。
37. AssetRegistry stable handle slice 已完成第一版：`AssetHandle` 使用 `asset:<kind>:<source>:<path>` 格式，scene package 写出 mesh / light / legacy-object `adapter.assetHandle` 并保留 `adapter.assetReference` fallback。
38. Camera adapter 当前只保存 component type / adapter kind，不写可解析 handle/reference；后续需要先定义 camera ownership/resolver 策略。
39. Scene package typed actor restore 已完成第一版：load 阶段恢复 `MeshActor`、`LightActor`、`LegacyObjectActor` 等动态 actor 类型，并由 verification 断言 `loadedTypedActors=6`。
40. Scene package negative probes 已完成第一版：loader 会统计 unknown actor/component type 与 invalid transform，并由 verification 断言损坏 package 拒绝和 unknown type fallback。
41. Imported asset stable handle coverage 已完成第一版：`LegacySceneWorldBuilder` 可在导入时向 `AssetRegistry` 注册 imported mesh/material/texture handles，并由 `--verify-pbr-import` 断言三类 handle 均存在。
42. Imported asset scene package manifest 已完成第一版：scene package 会写出 `assetManifest.N.*` 并读回 imported mesh/material/texture handles。
43. Editor asset browser / inspector 可见层已完成第一版：`AppRuntimeContext` 持有 runtime `AssetRegistry`，`EditorPanelContext` 只读引用 registry，asset selection 使用 handle 字符串并在 inspector 中查询 descriptor。
44. Scene package graph validation 已完成第一版：loader 会在 attach 前拒绝 duplicate persistent id、unresolved parent、invalid parent index、self-parent 和 cycle parent，并统计合法 cross-actor parent restore。
45. Scene package asset manifest registry reload 已完成第一版：`ScenePackageLoadOptions::assetRegistry` 可把 package manifest 注册回 runtime `AssetRegistry`。
46. AssetRegistry subsystem boundary 已完成第一版：`AssetSubsystem : EngineSubsystem` 拥有 `AssetRegistry`，runtime context 不再暴露裸 `AssetRegistry` 字段。
47. Engine-owned AssetSubsystem handoff 已完成第一版：`RuntimeApplicationShell` 通过 `Engine::addSubsystem<AssetSubsystem>()` 创建资产子系统，runtime context 只保存非拥有 `AssetSubsystem*`。
48. Engine-owned World handoff 已完成第一版：`AppRuntimeContext::engineWorld` 改为非拥有 `World*`，scene setup pipeline 通过 `Engine::createWorld(...)` 创建 runtime active World。
49. Engine tick frame loop 已完成第一版：`RuntimeFrameRunner` 在旧 renderer frame pipeline 前调用 `Engine::tick(deltaSeconds)`，active World 会 begin play 并参与 tick。
50. Runtime frame clock 已完成第一版：普通运行通过 `RuntimeFrameClock` 计算真实 delta，verification 模式使用固定 `1/60s`。
51. RendererSubsystem frame bridge 已完成第一版：`RuntimeApplicationShell` 通过 Engine 创建 renderer subsystem，scene prepare 后绑定 runtime renderer，frame runner 在旧 renderer pipeline 前后记录 begin/end，并由 verification 断言 observed renderer passes。
52. RendererSubsystem render entry wrapper 已完成第一版：`RuntimeFrameRunner` 通过 `RendererSubsystem::renderFrameBridge(...)` 包住旧 `RuntimeFramePipeline::render(...)`，verification 断言 `renderFrameBridgeCalls == beginFrames`。
53. RendererSubsystem editor stats visibility 已完成第一版：`AppRuntimeContext` 暴露非拥有 `RendererSubsystem*`，Debug controller 显示 subsystem bridge stats，verification 断言 context 指针与 Engine-owned subsystem 一致。
54. RendererSubsystem frame entry intent 已完成第一版：`renderFrameBridge(...)` 接收 `RendererFrameIntent`，记录 framebuffer size，并由 verification 断言 `frameConfigValid=yes`。
55. RendererSubsystem neutral frame pass stats 已完成第一版：`RuntimeFramePipeline::render(...)` 返回 planned/executed/skipped 统计，application adapter 转成 `RendererFrameResult` 中的 pass count，verification 断言 planned == executed + skipped。
56. Runtime renderer frame bridge adapter 已完成第一版：application 层 adapter 集中负责旧 `RuntimeFramePipeline` 调用和 stats 映射，Runner 不再直接依赖 runtime pipeline 细节。
57. Frame plan key 已完成第一版：adapter 生成非路径 `framePlanKey`，RendererSubsystem stats、verification 和 debug UI 均可观察该 render intent token，Engine DTO 不再暴露 `runtimePipeline*` 字段名。
58. RendererSubsystem frame intent/result DTO 已完成第一版：`RendererSubsystemFrameConfig` / `RendererSubsystemFrameRenderResult` 已整理为 `RendererFrameIntent` / `RendererFrameResult`。
59. Engine diagnostics panel 已完成第一版：RendererSubsystem frame bridge diagnostics 从 DebugControllerPanel 迁入独立 `EngineDiagnosticsPanel`。
60. Engine diagnostics unified context 已完成第一版：`AppRuntimeContext` 暴露非拥有 `Engine*`，Engine diagnostics panel 同时观察 Engine runtime、World、AssetSubsystem 和 RendererSubsystem 基础状态，verification 断言 `runtimeContextEngineAttached=yes`。
61. Renderer frame contract neutralization 已完成第一版：Engine-side DTO / stats 使用 `framePlanKey` 与 `plannedPassCount` / `executedPassCount` / `skippedPassCount`，旧 runtime pipeline 细节保留在 application adapter。
62. Engine-driven subsystem health counters 已完成第一版：Engine / World / AssetSubsystem / RendererSubsystem 都记录 tick count，verification 断言四者大于 0 且一致，并确认 runtime context World 匹配 active World。
63. Renderer backend interface 已完成第一版：`RendererSubsystem::renderFrameBridge(...)` 依赖 `RendererBackend`，application adapter 实现 backend contract 并调用旧 runtime pipeline，verification 断言 `frameExecutorAttached=yes` 且 executor call count 与 bridge call count 一致。
64. Renderer backend attachment boundary 已完成第一版：`RendererSubsystem` 持有 engine-owned `RendererBackend`，shell 负责创建并移交，runner 每帧只提交 frame intent，不再构造 executor。
65. Renderer backend metadata contract 已完成第一版：`RendererBackend` 暴露 backend key / readiness，`RendererSubsystemFrameBridgeStats` 记录 `rendererBackendReady` 与 `rendererBackendKey`，verification 断言 backend ready 且 key 非 `none`。
66. Renderer backend lifecycle stats 已完成第一版：RendererSubsystem stats 记录 backend state、attach/detach count、ready/not-ready frame count，verification 断言正常 runtime path 中 backend state 为 ready 且 ready frame count 覆盖所有 render bridge call。
67. Runtime renderer backend factory 已完成第一版：`RuntimeApplicationShell` 通过 `RuntimeRendererBackendFactory` 创建 `RendererBackend`，再移交给 Engine-owned `RendererSubsystem`。
68. Runtime renderer backend registry 已完成第一版：`RuntimeApplicationShellConfig::rendererBackendKey` 通过 factory registry 选择 backend，RendererSubsystem stats 记录 `rendererBackendRegistryKey` 与 `rendererBackendRegistryCount`。
69. Renderer backend registry no-op verification 已完成第一版：`test-noop-renderer-backend` 证明 registry 能切换到不调用旧 `RuntimeFramePipeline` 的第二个 backend，且 verification 断言 selected backend key / registry key / neutral frame result / legacy renderer pass count。
70. Renderer backend Engine-owned ownership 已完成第一版：`RendererSubsystem` 通过 `std::unique_ptr<RendererBackend>` 持有 active backend，verification 断言 `rendererBackendOwnerKey=engine-renderer-subsystem` 与 `rendererBackendOwnership=engine-owned`。
71. Renderer backend cleanup verification 已完成第一版：verification 断言 cleanup 阶段 `frameExecutorAttached=no`、`rendererBackendState=detached`、backend key / owner / registry metadata 已清空，且 attach/detach count 闭合。
72. Engine World cleanup verification 已完成第一版：verification 断言 cleanup 后 `engineInitialized=no`、`activeWorld=no`，且 runtime context 中 Engine / World / AssetSubsystem / RendererSubsystem 非拥有指针均已清空。
73. Engine Subsystem cleanup verification 已完成第一版：verification 断言 post-shutdown `assetSubsystemInitialized=no`、`assetRegistryAssets=0`、`rendererSubsystemInitialized=no`、`rendererHasRenderer=no`、`rendererFrameExecutorAttached=no`。
74. Runtime Engine Lifecycle extraction 已完成第一版：`RuntimeEngineLifecycle` 集中 `initializeEngine`、`attachRendererBackend`、`beginCleanup`、`detachRuntimeContext` 和 `shutdownEngine`，`RuntimeApplicationShell` 改为持有 `RuntimeEngineLifecycleState`。
75. Runtime Verification Lifecycle extraction 已完成第一版：`RuntimeVerificationLifecycle` 集中 verification profile、scene report、capture、cleanup report 和 max-frame stop 条件，`RuntimeApplicationShell` 不再直接调用 `RuntimePBRVerification::*`。
76. Runtime Content Lifecycle extraction 已完成第一版：`RuntimeContentLifecycle` 集中 camera/profile/verification startup/scene prepare/backend attach/prepared scene report 的高层编排，`RuntimeApplicationShell` 只保留 content config 聚合。
77. Runtime Content Config Policy extraction 已完成第一版：`RuntimeContentConfigPolicy` 集中 shell config 到 `RuntimeContentLifecycleConfig` 的映射，`RuntimeApplicationShell` 不再直接知道 verification scene flags 如何改写 scene setup pipeline。
78. Runtime Frame Lifecycle extraction 已完成第一版：`RuntimeFrameLifecycle` 集中 frame clock、frame runner、rendered frame count、verification capture flag 和 max-frame continue 条件，`RuntimeApplicationShell` 不再直接保存 frame lifecycle 状态。
79. Runtime Editor Lifecycle extraction 已完成第一版：`RuntimeEditorLifecycle` 集中 GUI 初始化、editor panel frame callback、selection state 和 edit transaction state，`RuntimeApplicationShell` 不再直接依赖 `RuntimeGuiHost` / `RuntimeEditorPanelCoordinator`。
80. Runtime Graphics Lifecycle extraction 已完成第一版：`RuntimeGraphicsLifecycle` 集中窗口创建后的 viewport 初始化、clear color 设置和 OpenGL capability 输出，`RuntimeApplicationShell` 不再直接依赖 OpenGL 诊断/viewport 细节。
81. Engine Lifecycle Snapshot 已完成第一版：`EngineLifecycleSnapshot` 与 `Engine::captureLifecycleSnapshot()` 集中 Engine initialized/run mode/viewport/time/tick/subsystem/active World 观察字段，Editor diagnostics 与 verification 共用同一快照。
82. Runtime Application Config Policy extraction 已完成第一版：`RuntimeApplicationConfigPolicy` 集中 Engine desc、frame/editor/graphics lifecycle config 的非 content 映射，`RuntimeApplicationShell` 不再保留对应私有 `make*Config` 成员。
83. Runtime Window Lifecycle Snapshot Boundary 已完成第一版：`RuntimeWindowLifecycle` 提供当前窗口快照和销毁入口，`RuntimeApplicationShell` 不再直接访问 `Application` 单例的 width/height/window/destroy。
84. Engine Subsystem Lifecycle Summary Snapshot 已完成第一版：`EngineSubsystem` 提供 `getDebugName()` / initialized / tick count 诊断接口，`EngineLifecycleSnapshot` 可枚举 Engine-owned subsystem summary，diagnostics 与 verification 共用该 summary。
85. Runtime Application Startup Lifecycle extraction 已完成第一版：`RuntimeApplicationStartupLifecycle` 集中 Engine / Window / Graphics / Content / Editor / Frame reset 启动顺序，`RuntimeApplicationShell::initialize()` 只保留 startup 委托。
86. Runtime Application Shutdown Lifecycle extraction 已完成第一版：`RuntimeApplicationShutdownLifecycle` 集中 renderer cleanup report、camera cleanup、runtime context detach、Engine shutdown、Engine cleanup report 和 window destroy 顺序，`RuntimeApplicationShell::cleanup()` / `destroy()` 只保留 shutdown 委托。
87. Runtime Application Frame Lifecycle extraction 已完成第一版：`RuntimeApplicationFrameLifecycle` 集中 frame continue、window snapshot、frame/editor config mapping、editor frame callbacks 和 `RuntimeFrameLifecycle` 调用，`RuntimeApplicationShell::shouldContinue()` / `runFrame()` 只保留 frame 委托。
88. Runtime Application State Context extraction 已完成第一版：`RuntimeApplicationState` 集中 Engine、runtime context、engine/editor/frame lifecycle state 和 legacy experiments，`RuntimeApplicationShell` 只保留 state 与 config。
89. Renderer backend contract verification naming neutralization 已完成第一版：verification 并行输出 `Runtime renderer backend contract stats` / cleanup stats 与旧 subsystem stats，脚本断言 backend attach/ready、registry key/count、frame pass counts、no-op backend 行为和 cleanup detach 状态。
90. Renderer backend contract header extraction 已完成第一版：`RendererBackend.h` 定义 `RendererBackend`、`RendererFrameIntent`、`RendererFrameResult` 与 attachment desc，`RendererSubsystem.h` 只消费该 contract；application backend/factory 不再直接依赖 subsystem header。
91. Renderer backend registry metadata contract extraction 已完成第一版：`RendererBackendRegistration` 定义在 engine-level `RendererBackend.h`，runtime factory 的 `registeredBackends()` 返回 engine-level registration entry。
92. Renderer backend registry helper extraction 已完成第一版：`RendererBackendRegistry` 集中 registry query、default backend key 选择和 attachment desc 组装，runtime factory 只保留 runtime backend 列表和具体 backend 创建。
93. Renderer backend registry selection policy extraction 已完成第一版：`RendererBackendSelection` 与 `resolveBackendSelection(...)` 集中 requested/default/selected/registered 语义，runtime factory 只按 selected key 创建具体 application backend。
94. Runtime renderer backend catalog extraction 已完成第一版：`RuntimeRendererBackendCatalog` 集中 runtime backend key、registry metadata、selection entrypoint 和 attachment metadata；`RuntimeRendererBackendFactory` 只负责按 `RendererBackendSelection` 创建具体 backend object。
95. Runtime Application Config Header extraction 已完成第一版：`RuntimeApplicationShellConfig` 独立到 `RuntimeApplicationConfig.h`，config policy/content policy/startup-frame-shutdown lifecycle/verification args 不再为了 config 包含完整 `RuntimeApplicationShell.h`。
96. Runtime Application Callback Binder extraction 已完成第一版：`RuntimeApplicationCallbackBinder` 集中 bootstrapper callbacks 到 startup/frame/shutdown lifecycle 的绑定，`RuntimeApplicationShell` 不再保留私有 lifecycle wrapper。
97. Runtime Application Runner Boundary extraction 已完成第一版：`RuntimeApplicationRunner` 集中 `RuntimeApplicationShell` 构造、callback 获取和 `RuntimeBootstrapper::run(...)` 调用，`main.cpp` 不再直接依赖 Shell 或 Bootstrapper。
98. Runtime Application Entry Boundary extraction 已完成第一版：`RuntimeApplicationEntry` 集中 logger setup、verification args parsing 和 runner 调用；`PointLightShadow::MAX_POINT_LIGHTS` 静态定义已迁回 light module，`main.cpp` 只保留 entry 委托。
99. Runtime Verification Args Neutral Alias extraction 已完成第一版：新增 `RuntimeVerificationArgs.h` 作为 entry-facing neutral public header，`RuntimeApplicationEntry` 不再直接包含 `RuntimePBRVerificationArgs.h`；旧 PBR args header 保留为兼容 wrapper，`.cpp` 实现暂不搬迁。
100. Runtime Verification Args Implementation extraction 已完成第一版：`makeShellConfigFromArguments(...)` 实现已迁入 `RuntimeVerificationArgs.cpp`，旧 `RuntimePBRVerificationArgs.cpp` 只保留兼容 stub，内部 descriptor/table 命名已从 PBR-specific 收敛为 runtime verification 语义。
101. Runtime Verification Config Field Neutralization 已完成第一版：新增 `RuntimeVerificationConfig` neutral alias，`RuntimeApplicationShellConfig` 字段从 `pbrVerification` 改为 `verification`，application config/content/frame/shutdown/verification lifecycle 与 args parser 均改为消费 `config.verification`。
102. Runtime Verification Config Data Model Split 已完成第一版：`RuntimeVerificationConfig` 不再是 `RuntimePBRVerificationConfig` alias，generic lifecycle/capture 字段留在 runtime verification 顶层，PBR probe、Engine World probe 和 renderer timing probe 分别进入 `pbr`、`engineWorld`、`renderer` 子配置。
103. Runtime Verification Report extraction 已完成第一版：新增 `RuntimeVerificationReport`，把 Engine lifecycle snapshot、subsystem health、renderer backend contract 和 cleanup report 从 `RuntimePBRVerification` 迁出；PBR 文件只保留 PBR renderer stats report。
104. Runtime Engine World Verification extraction 已完成第一版：新增 `RuntimeEngineWorldVerification`，把 editor-create probe、transform snapshot、scene package round-trip、negative package probes 和 package resolver fixture 从 `RuntimePBRVerification` 迁出；PBR 文件保留 PBR scene/import/renderer stats。
105. Runtime Engine World Prepared Scene Stats Neutralization 已完成第一版：`PBR verification scene stats` 不再输出 `engineWorldProbeMeshes`、`engineWorldMinimalMeshes` 或 `runtimeWorldActors`；这些 fields 由 `Engine world prepared scene stats` 中性行输出并由脚本断言。
106. Runtime Imported Asset Verification extraction 已完成第一版：新增 `RuntimeImportedAssetVerification`，把 imported asset 的 Engine World import、AssetRegistry stats 和 scene package manifest round-trip 从 `RuntimePBRVerification` 迁出，保留既有 `--verify-pbr-import` 输出契约。
107. Runtime Verification Capture extraction 已完成第一版：新增 `RuntimeVerificationCapture`，把 default framebuffer readback、PPM 写盘和 capture 日志从 `RuntimePBRVerification` 迁出，verification lifecycle 不再通过 PBR 类执行 capture。
108. Runtime PBR Renderer Stats Verification extraction 已完成第一版：新增 `RuntimePBRRendererStatsVerification`，把 capture 帧的 `PBR verification renderer stats` 输出从 `RuntimePBRVerification` 迁出，保持既有 renderer stats 输出契约。
109. Runtime PBR Profile Verification extraction 已完成第一版：新增 `RuntimePBRProfileVerification`，把 PBR startup profile、light/camera rig、frame pipeline profile 和 renderer pass profile 写入从 `RuntimePBRVerification` 迁出。
110. Runtime PBR Scene Probe Verification extraction 已完成第一版：新增 `RuntimePBRSceneProbeVerification`，把 transparent fallback、deferred emissive、material IBL、alpha mask、texture set 和 showcase spheres 的 PBR scene probe construction 从 `RuntimePBRVerification` 迁出。
111. Runtime PBR Prepared Scene Stats Verification extraction 已完成第一版：新增 `RuntimePBRPreparedSceneStatsVerification`，把 `PBR verification scene stats` 从旧 `RuntimePBRVerification` class 中迁出；旧 `.cpp` 已从工程移除。
112. Runtime PBR Verification Config Header Rename 已完成第一版：新增 `RuntimePBRVerificationConfig.h` 承载 `RuntimePBRVerificationConfig`，`RuntimeVerificationConfig.h` 改为包含明确命名的 config header；旧 `RuntimePBRVerification.h` 已从 live code 与 VS 工程注册中移除。
113. Runtime PBR Verification Config Pass/Probe/Deferred Split 已完成第一版：`RuntimePBRVerificationConfig` 现在聚合 `passes`、`probes`、`deferred` 三个子配置，args/profile/probe/import verification 不再依赖单个 flat PBR config 字段列表。
114. Runtime PBR Pass Profile Verification extraction 已完成第一版：新增 `RuntimePBRPassProfileVerification`，把 renderer pass profile 写入从 `RuntimePBRProfileVerification` 中拆出；prepared-scene lifecycle 可直接刷新 renderer pass profile。
115. Runtime PBR Profile Line Verification extraction 已完成第一版：新增 `RuntimePBRProfileLineVerification`，把 `PBR verification profile applied` 输出行从 startup profile 写入中拆出，profile module 不再直接依赖 logger/stdout。
116. Runtime PBR Preview Profile Verification extraction 已完成第一版：新增 `RuntimePBRPreviewProfileVerification`，把 PBR preview grid policy 从 startup profile 写入中拆出，保持默认 grid、showcase override 和 minimal-scene disable 行为不变。
117. Runtime PBR Light Camera Rig Verification extraction 已完成第一版：新增 `RuntimePBRLightCameraRigVerification`，把 PBR verification 默认、minimal-scene、tiled-light、showcase-camera 和 pressure-light rig policy 从 startup profile composition 中拆出。
118. Runtime PBR Startup Profile Verification extraction 已完成第一版：新增 `RuntimePBRStartupProfileVerification`，把 environment、post-process 和 runtime frame pipeline startup defaults 从 `RuntimePBRProfileVerification` 中拆出；`RuntimePBRProfileVerification` 当前只保留 profile application 编排。
119. Runtime Content Lifecycle Config Header extraction 已完成第一版：新增 `RuntimeCameraConfig.h`、`RuntimeScenePrepareConfig.h` 与 `RuntimeContentLifecycleConfig.h`，让 `RuntimeContentLifecycle.h` 不再暴露 camera lifecycle、scene preparer、engine lifecycle、verification lifecycle 或 legacy experiment implementation headers。
120. Runtime Content Renderer Backend Lifecycle extraction 已完成第一版：新增 `RuntimeContentRendererBackendLifecycle`，把 scene prepare 后的 renderer fail-fast gate 与 Engine-owned renderer backend attachment 从 generic content composition 中拆出；`RuntimeContentLifecycle.cpp` 不再直接依赖 `RuntimeEngineLifecycle.h` 或 logger。
121. Runtime Content Verification Lifecycle extraction 已完成第一版：新增 `RuntimeContentVerificationLifecycle`，把 runtime profile load、verification startup profile 和 prepared-scene report 从 generic content composition 中拆出；`RuntimeContentLifecycle.cpp` 不再直接依赖 `RuntimeProfileLoader.h` 或 `RuntimeVerificationLifecycle.h`。
122. Runtime Content Scene Lifecycle extraction 已完成第一版：新增 `RuntimeContentSceneLifecycle`，把 scene preparation stage 从 generic content composition 中拆出；`RuntimeContentLifecycle.cpp` 不再直接依赖 `RuntimeScenePreparer.h`。
123. Runtime Content Camera Lifecycle extraction 已完成第一版：新增 `RuntimeContentCameraLifecycle`，把 startup camera initialization stage 从 generic content composition 中拆出；`RuntimeContentLifecycle.cpp` 不再直接依赖 `RuntimeCameraLifecycle.h`。
124. Runtime Legacy Experiment Lifecycle extraction 已完成第一版：新增 `RuntimeLegacyExperimentLifecycle`，把 legacy experiment context construction、startup enable hooks 和 per-frame update 从 `RuntimeScenePreparer` 中拆出；`RuntimeFrameRunner.cpp` 不再为了 legacy update 依赖 `RuntimeScenePreparer.h`。
125. Scene Setup Pipeline Config Header extraction 已完成第一版：新增 `SceneSetupPipelineConfig.h`，让 `RuntimeScenePrepareConfig.h` 只依赖轻量 pipeline config DTO，不再 public include 完整 `SceneSetupPipeline.h`。
126. Runtime Scene Setup Report extraction 已完成第一版：新增 `RuntimeSceneSetupReport`，把 scene setup result stdout/logger 输出和 renderer prepared 日志从 `RuntimeScenePreparer` 中拆出；`RuntimeScenePreparer.cpp` 不再直接依赖 logger/stdout 或 scene setup stats formatter。
127. Runtime Scene Setup Context Factory extraction 已完成第一版：新增 `RuntimeSceneSetupContextFactory`，把 `AppRuntimeContext` 到 `GL_SCENE::SetupContext` 的字段展开从 `RuntimeScenePreparer` 中拆出；`RuntimeScenePreparer` 不再公开 `makeSceneSetupContext(...)`。
128. Runtime Scene Setup Pipeline Lifecycle extraction 已完成第一版：新增 `RuntimeSceneSetupPipelineLifecycle`，把 setup context factory、`GL_SCENE::prepareScene(...)` 和 prepared-scene setup report 从 `RuntimeScenePreparer` 中拆出；`RuntimeScenePreparer.cpp` 不再直接依赖完整 `SceneSetupPipeline.h`。
129. Runtime Scene Preparer removal 已完成第一版：删除无状态 `RuntimeScenePreparer.h/.cpp` wrapper，`RuntimeContentSceneLifecycle` 直接拥有 scene setup pipeline lifecycle、legacy experiment lifecycle prepare 和 renderer prepared report 的顺序。
130. Runtime Application Content Startup Lifecycle extraction 已完成第一版：新增 `RuntimeApplicationContentStartupLifecycle`，把 startup lifecycle 中 content config policy 与 `RuntimeContentLifecycle::prepare(...)` 桥接拆出；`RuntimeApplicationStartupLifecycle.cpp` 不再直接依赖 content config policy 或 content lifecycle。
131. Runtime Application Editor Startup Lifecycle extraction 已完成第一版：新增 `RuntimeApplicationEditorStartupLifecycle`，把 startup lifecycle 中 editor config policy 与 `RuntimeEditorLifecycle::initialize(...)` 桥接拆出；`RuntimeApplicationStartupLifecycle.cpp` 不再直接依赖 editor lifecycle。
132. Runtime Application Frame Startup Lifecycle extraction 已完成第一版：新增 `RuntimeApplicationFrameStartupLifecycle`，把 startup lifecycle 中 `RuntimeFrameLifecycle::reset(...)` 桥接拆出；`RuntimeApplicationStartupLifecycle.cpp` 不再直接 reset frame lifecycle。
133. Runtime Application Engine Startup Lifecycle extraction 已完成第一版：新增 `RuntimeApplicationEngineStartupLifecycle`，把 startup lifecycle 中 Engine desc mapping 与 `RuntimeEngineLifecycle::initializeEngine(...)` 桥接拆出；`RuntimeApplicationStartupLifecycle.cpp` 不再直接初始化 Engine lifecycle。
134. Runtime Application Window Startup Lifecycle extraction 已完成第一版：新增 `RuntimeApplicationWindowStartupLifecycle`，把 startup lifecycle 中 window setup prompt、`RuntimeWindowLifecycle::initialize(...)` 与 `RuntimeWindowLifecycle::captureSnapshot()` 桥接拆出；`RuntimeApplicationStartupLifecycle.cpp` 不再直接初始化或读取 window lifecycle。
135. Runtime Application Graphics Startup Lifecycle extraction 已完成第一版：新增 `RuntimeApplicationGraphicsStartupLifecycle`，把 startup lifecycle 中 graphics config mapping 与 `RuntimeGraphicsLifecycle::initializeAfterWindow(...)` 桥接拆出；`RuntimeApplicationStartupLifecycle.cpp` 不再直接初始化 graphics lifecycle。
136. Runtime Application Frame Editor Callback Bridge extraction 已完成第一版：新增 `RuntimeApplicationFrameEditorCallbackBridge`，把 frame lifecycle 中 editor callback config mapping 与 `RuntimeEditorLifecycle::makeFrameCallbacks(...)` 桥接拆出；`RuntimeApplicationFrameLifecycle.cpp` 不再直接创建 editor frame callbacks。
137. Runtime Application Frame Continue/Run Bridge extraction 已完成第一版：新增 `RuntimeApplicationFrameContinueBridge` 与 `RuntimeApplicationFrameRunBridge`，把 frame lifecycle 中 continue config mapping、window snapshot、frame execution 参数展开和 `RuntimeFrameLifecycle` 调用拆出；`RuntimeApplicationFrameLifecycle.cpp` 当前只保留 facade 转发。
138. Runtime Application Shutdown Cleanup/Destroy Bridge extraction 已完成第一版：新增 `RuntimeApplicationShutdownCleanupBridge` 与 `RuntimeApplicationShutdownDestroyBridge`，把 shutdown lifecycle 中 cleanup sequence 和 window destroy 细节拆出；`RuntimeApplicationShutdownLifecycle.cpp` 当前只保留 facade 转发。
139. Runtime Application Shutdown Verification Bridge extraction 已完成第一版：新增 `RuntimeApplicationShutdownVerificationBridge`，把 shutdown cleanup 中 renderer subsystem cleanup report 与 Engine cleanup report 从 cleanup sequence bridge 中拆出；`RuntimeApplicationShutdownCleanupBridge.cpp` 不再直接依赖 `RuntimeVerificationLifecycle`。
140. Runtime Application Shutdown Engine Bridge extraction 已完成第一版：新增 `RuntimeApplicationShutdownEngineBridge`，把 shutdown cleanup 中 begin cleanup、camera cleanup、runtime context detach 和 Engine shutdown 从 cleanup sequence bridge 中拆出；早期 application-level cleanup refs DTO 已在后续 canonicalization 中由 `RuntimeEngineLifecycleCleanupRefs` 取代。
141. Runtime Engine Lifecycle Types Header extraction 已完成第一版：最初新增 `RuntimeEngineLifecycleTypes.h`，把 `RuntimeEngineLifecycleState` 与 `RuntimeEngineLifecycleCleanupRefs` 从 `RuntimeEngineLifecycle.h` 中拆出；当前已进一步被 `RuntimeEngineLifecycleState.h` 与 `RuntimeEngineLifecycleCleanupRefs.h` 两个窄头取代。
142. Runtime Application State Forward Boundary 已完成第一版：`RuntimeApplicationShell.h` 不再 public include `RuntimeApplicationState.h`，并通过 `std::unique_ptr<RuntimeApplicationState>` 隐藏完整 state 类型；application startup/frame/shutdown bridge headers 改为前置声明 state/config，完整依赖局部化到 `.cpp`。
143. Runtime Profile State extraction 已完成第一版：新增 `RuntimeProfileState.h`，把 frame pipeline、post-process、environment、PBR preview/light/camera profile 与 profile paths 从 `AppRuntimeContext` 中聚合到 `context.profiles`；`AppRuntimeContext.h` 不再直接 include 这些 profile headers。
144. Runtime Render Resource State extraction 已完成第一版：新增 `RuntimeRenderResourceState.h`，把 renderer、screen/world scenes、frame render targets、bloom、runtime meshes/materials、post-process pass 与 clear color 从 `AppRuntimeContext` 中聚合到 `context.renderResources`；`AppRuntimeContext.h` 不再直接持有这些 top-level render resource 字段。
145. Runtime Camera Light State extraction 已完成第一版：新增 `RuntimeCameraLightState.h`，把 camera、camera control、ambient/directional/spot/point light state 从 `AppRuntimeContext` 中聚合到 `context.cameraLights`；`AppRuntimeContext.h` 不再直接持有这些 top-level camera/light 字段。
146. Runtime Engine Attachment State extraction 已完成第一版：新增 `RuntimeEngineAttachmentState.h`，把 Engine、World、AssetSubsystem、RendererSubsystem 与 engine-world editable flag 从 `AppRuntimeContext` 中聚合到 `context.engineAttachments`；`AppRuntimeContext.h` 不再直接声明这些 engine/runtime attachment 字段。
147. Runtime Renderer Backend Attachment Lifecycle extraction 已完成第一版：新增 `RuntimeRendererBackendAttachmentLifecycle`，把 renderer backend selection、factory create、attachment desc 组装和 `RendererSubsystem::setRendererBackend(...)` 从 `RuntimeEngineLifecycle.cpp` 中拆出。
148. RendererSubsystem Frame Bridge Stats Header extraction 已完成第一版：新增 `RendererSubsystemFrameBridgeStats.h`，把 frame bridge stats DTO 从 `RendererSubsystem.h` 中拆出，保持 `RendererSubsystem::getFrameBridgeStats()` 合同不变。
149. RendererSubsystem Frame Bridge State extraction 已完成第一版：新增 `RendererSubsystemFrameBridgeState`，集中 frame bridge stats 写入策略，保持 `RendererSubsystem::getFrameBridgeStats()` 合同和 verification 输出不变。
150. RendererSubsystem Backend Slot extraction 已完成第一版：新增 `RendererSubsystemBackendSlot`，把 renderer backend pointer storage、attachment metadata normalization、backend ready 判断和 attach/detach change detection 从 `RendererSubsystem` 中拆出。
151. RendererSubsystem Backend Slot Snapshot extraction 已完成第一版：新增 `RendererSubsystemBackendSlotSnapshot`，让 `RendererSubsystemFrameBridgeState` 通过只读 snapshot 刷新 backend stats，不再直接探测 raw `RendererBackend*`。
152. RendererSubsystem Frame Execution Bridge extraction 已完成第一版：新增 `RendererSubsystemFrameExecutionBridge`，把 backend frame execution / default frame result generation 从 `RendererSubsystem` 中拆出，保持 lifecycle、stats counter 顺序和 verification 输出不变。
153. RendererSubsystem Implementation State Header Boundary Cleanup 已完成第一版：`RendererSubsystem.h` 用 private owning pointers 隐藏 backend slot、frame execution bridge 和 frame bridge state 实现成员，完整实现头依赖下沉到 `RendererSubsystem.cpp`；需要读取 stats 或析构 backend 的调用点改为显式 include 对应完整类型头。
154. RendererSubsystem Renderer Backend API Naming Cleanup 已完成第一版：`RendererSubsystem` 新增 `setRendererBackend(...)` / `clearRendererBackend()` / `getRendererBackend()` / `hasRendererBackend()`，live application code 改用 renderer backend 命名，并为旧 verification 字段与新字段建立同步。
155. RendererSubsystem Renderer Backend Public Compatibility API Removal 已完成第一版：删除 `RendererFrameExecutor`、`RendererSubsystemFrameIntent`、`RendererSubsystemFrameResult` aliases，以及 `setFrameExecutor(...)` / `hasFrameExecutor()` 等 public wrappers；旧 `frameExecutor*` 只作为 verification/log compatibility 字段保留。
156. RendererSubsystem Frame Executor Stats Internal Compatibility Removal 已完成第一版：`RendererSubsystemFrameBridgeStats` 不再保存 `frameExecutorAttached` / `frameExecutorCallCount`，旧 `frameExecutor*` 输出由 `rendererBackend*` stats 派生，仅作为 verification/log compatibility 字段保留。
157. RendererSubsystem Frame Executor Verification Output Removal 已完成第一版：`RuntimeVerificationReport` 与 `verify_pbr.ps1` 不再输出、解析或断言旧 `frameExecutor*` 字段，renderer backend contract 只暴露 `rendererBackend*` verification 字段。
158. Runtime Renderer Backend Verification Report Formatter extraction 已完成第一版：新增 `RuntimeRendererBackendVerificationReport`，把 renderer backend verification report line formatting 从 `RuntimeVerificationReport` 中拆出，保持四类 runtime renderer backend 输出合同不变。
159. Runtime Engine Verification Report Formatter extraction 已完成第一版：新增 `RuntimeEngineVerificationReport`，把 Engine lifecycle snapshot、subsystem summary、tick health、Engine World cleanup 和 Engine subsystem cleanup 行格式化从 `RuntimeVerificationReport` 中拆出，保持 report 输出合同不变。
160. Runtime Verification Frame Capture Lifecycle extraction 已完成第一版：新增 `RuntimeVerificationFrameCaptureLifecycle`，把 framebuffer capture、runtime Engine/renderer report 和 PBR renderer stats report 编排从 `RuntimeVerificationLifecycle` 中拆出，保持 public API 与输出合同不变。
161. Runtime Verification Prepared Scene Lifecycle extraction 已完成第一版：新增 `RuntimeVerificationPreparedSceneLifecycle`，把 Engine World probe、PBR scene probe、imported asset probe、renderer pass profile 和 prepared-scene stats report 编排从 `RuntimeVerificationLifecycle` 中拆出，保持 public API、输出顺序与 verification 合同不变。
162. Runtime Verification Cleanup Lifecycle extraction 已完成第一版：新增 `RuntimeVerificationCleanupLifecycle`，把 renderer subsystem cleanup、Engine World cleanup 和 Engine subsystem cleanup report 编排从 `RuntimeVerificationLifecycle` 中拆出，保持 public API、cleanup ordering 与 verification 输出合同不变。
163. Runtime Verification Startup Profile Lifecycle extraction 已完成第一版：新增 `RuntimeVerificationStartupProfileLifecycle`，把 startup profile gate 与 PBR profile application 从 `RuntimeVerificationLifecycle` 中拆出，保持 public API、profile applied 输出与 verification 合同不变。
164. Runtime Verification Stop Policy extraction 已完成第一版：新增 `RuntimeVerificationStopPolicy`，把 max-frame verification stop condition 从 `RuntimeVerificationLifecycle` 中拆出，保持 public API、frame loop 和 verification max-frame 语义不变。
165. Runtime Verification Lifecycle Header Forward Boundary 已完成第一版：`RuntimeVerificationLifecycle.h` 不再 include `AppRuntimeContext.h` 或 `RuntimeVerificationConfig.h`，只通过 forward declarations 暴露 facade API；需要完整 config 的 `RuntimeFrameLifecycle.h` 改为显式 include。
166. Runtime Frame Callbacks Header extraction 已完成第一版：新增 `RuntimeFrameCallbacks.h`，让 frame lifecycle、frame runner 与 editor callback bridge 共用轻量 callback DTO，`RuntimeFrameLifecycle.h` 不再为了 callback 类型传递 include runner/context/legacy experiment runner。
167. Runtime Frame Lifecycle Types Header extraction 已被后续窄头拆分取代：`RuntimeFrameLifecycleConfig` / `RuntimeFrameLifecycleState` 已从 `RuntimeFrameLifecycle.h` 中拆出，facade header 只保留行为入口和 forward declarations；当前 canonical 入口是 `RuntimeFrameLifecycleConfig.h` 与 `RuntimeFrameLifecycleState.h`。
168. Runtime Application Frame Editor Callback Bridge Header Boundary 已完成第一版：`RuntimeApplicationFrameEditorCallbackBridge.h` 改为 forward declare `RuntimeFrameCallbacks`，完整 callback DTO 依赖局部化到 `.cpp`。
169. Runtime Frame Runner Types Header extraction 已完成第一版：新增 `RuntimeFrameRunnerTypes.h`，把 `RuntimeFrameConfig` 从 `RuntimeFrameRunner.h` 中拆出，runner facade header 只保留 run entry、callback DTO 和 config forward declaration。
170. Runtime Application Frame Bridge Implementation Include Cleanup 已完成第一版：frame continue/run bridge headers 已确认为 forward boundary，facade/bridge `.cpp` 移除冗余 `RuntimeApplicationConfig.h` / `RuntimeApplicationState.h` include。
171. Runtime Application Shutdown Bridge Implementation Include Cleanup 已完成第一版：shutdown lifecycle facade `.cpp` 和 cleanup bridge `.cpp` 移除冗余完整 config/state include，verification bridge 保留真正读取 `config.verification` 所需 include。
172. Runtime Application Startup Bridge Include Surface Cleanup 已完成第一版：startup facade/content/editor/engine implementation 移除冗余完整 config/state include，graphics startup public header 改为 forward declare shell config。
173. Runtime Application Config Policy Header Include Surface Cleanup 已完成第一版：`RuntimeApplicationConfigPolicy.h` 改为 forward declaration boundary，完整 config/lifecycle/EngineContext 依赖局部化到 `.cpp` 与需要完整 `EngineDesc` 的调用点。
174. Runtime Content Config Policy Header Include Surface Cleanup 已完成第一版：`RuntimeContentConfigPolicy.h` 改为 forward declaration boundary，完整 shell/content config 依赖局部化到 `.cpp` 与显式消费返回临时对象的 content startup 调用点。
175. Runtime Content Lifecycle Header Config Forward Boundary Cleanup 已完成第一版：`RuntimeContentLifecycle.h` 改为 forward declare `RuntimeContentLifecycleConfig`，完整 config DTO 依赖局部化到 lifecycle implementation 与 content startup 调用点。
176. Runtime Content Camera/Scene Lifecycle Header Config Forward Boundary Cleanup 已完成第一版：`RuntimeContentCameraLifecycle.h` 与 `RuntimeContentSceneLifecycle.h` 改为 forward declare 对应 config DTO，完整 DTO 依赖局部化到各自 implementation。
177. Runtime Application Shutdown Cleanup Refs Canonicalization 已完成第一版：application shutdown bridge 不再定义重复的 cleanup refs DTO，直接复用 `RuntimeEngineLifecycleCleanupRefs` 作为 shutdown cleanup / verification report 的 canonical refs。
178. Runtime Engine Lifecycle Header Type Include Surface Cleanup 已完成第一版：`RuntimeEngineLifecycle.h` 改为 forward declare lifecycle state/cleanup refs；完整 state/cleanup refs 依赖当前已局部化到 implementation、state owner、frame 字段读取和 cleanup refs 字段访问方。
179. Runtime Renderer Backend Key String View Boundary Cleanup 已完成第一版：renderer backend key 在 content/engine/attachment/catalog/registry 只读传递路径中改为 `std::string_view`，config DTO、selection 和 attachment desc 仍保留 `std::string` 持久化字段。
180. Renderer Backend Registry Types Header Extraction 已完成第一版：新增 `RendererBackendRegistryTypes.h` 承载 attachment desc、registration 与 selection metadata；`RendererBackendRegistry.h` 与 runtime backend catalog 不再为了 registry metadata 依赖完整 `RendererBackend.h`。
181. Runtime Renderer Backend Keys Header Extraction 已完成第一版：新增 `RuntimeRendererBackendKeys.h` 承载 runtime/default/no-op backend key helper；config、verification args 与 factory implementation 不再为了 key 常量依赖 `RuntimeRendererBackendCatalog.h`。
182. Runtime Window Lifecycle Types Header Extraction 已完成第一版：新增 `RuntimeWindowLifecycleTypes.h` 承载 window config/snapshot/callback DTO；`RuntimeApplicationConfig.h` 不再为了 `RuntimeWindowConfig` 间接包含完整 `RuntimeWindowLifecycle.h` 和 `AppRuntimeContext.h`。
183. Runtime Frame Clock Config Header Extraction 已完成第一版：新增 `RuntimeFrameClockTypes.h` 承载 `RuntimeFrameClockConfig`；`RuntimeApplicationConfig.h` 不再为了 frame clock config 间接包含完整 `RuntimeFrameClock.h` 和 `<chrono>`。
184. Runtime Frame Lifecycle Config/State Header Split 已完成第一版：新增 `RuntimeFrameLifecycleConfig.h` 与 `RuntimeFrameLifecycleState.h`；只构造 frame lifecycle config 的路径不再通过旧 config/state 聚合入口间接包含 state、完整 frame clock 行为头和 `<chrono>`。
185. Runtime Editor Lifecycle Config/State Header Split 已完成第一版：新增 `RuntimeEditorLifecycleConfig.h` 与 `RuntimeEditorLifecycleState.h`；`RuntimeApplicationState.h` 不再为了 editor state 间接包含完整 `RuntimeEditorLifecycle.h` 行为头和 frame callback 依赖。
186. Runtime Application State Engine Owner Boundary Cleanup 已完成第一版：新增 `RuntimeApplicationState.cpp`，`RuntimeApplicationState.h` 改为 forward declare `GLengine::Engine` 并通过内部 `std::unique_ptr` + `engine()` accessor 拥有 Engine，完整 `Engine.h` 依赖局部化到 state implementation。
187. Runtime Application State Legacy Runner Owner Boundary Cleanup 已完成第一版：`RuntimeApplicationState.h` 改为 forward declare `GL_EXPERIMENTS::LegacyExperimentRunner` 并通过内部 `std::unique_ptr` + `legacyExperiments()` accessor 拥有 legacy runner，完整 `LegacyExperimentRunner.h` 依赖局部化到 state implementation。
188. Runtime Application State Runtime Context Owner Boundary Cleanup 已完成第一版：`RuntimeApplicationState.h` 改为 forward declare `GLframework::AppRuntimeContext` 并通过内部 `std::unique_ptr` + `runtime()` accessor 拥有 runtime context，完整 `AppRuntimeContext.h` 依赖局部化到 state implementation。
189. Runtime Application State Editor Lifecycle State Owner Boundary Cleanup 已完成第一版：`RuntimeApplicationState.h` 改为 forward declare `RuntimeEditorLifecycleState` 并通过内部 `std::unique_ptr` + `editorLifecycle()` accessor 拥有 editor lifecycle state，完整 `RuntimeEditorLifecycleState.h` / `EditorPanels.h` 依赖局部化到 state implementation 和 editor lifecycle implementation 路径。
190. Runtime Application State Frame Lifecycle State Owner Boundary Cleanup 已完成第一版：`RuntimeApplicationState.h` 改为 forward declare `RuntimeFrameLifecycleState` 并通过内部 `std::unique_ptr` + `frameLifecycle()` accessor 拥有 frame lifecycle state，完整 `RuntimeFrameLifecycleState.h` / `RuntimeFrameClock.h` / `<chrono>` 依赖局部化到 state implementation 和 frame lifecycle implementation 路径。
191. Runtime Application State Engine Lifecycle State Accessor Boundary Cleanup 已完成第一版：`RuntimeApplicationState.h` 改为 forward declare `RuntimeEngineLifecycleState` 并通过内部 `std::unique_ptr` + `engineLifecycle()` accessor 拥有 engine lifecycle state，完整 state 依赖局部化到 state implementation 和 frame run 字段读取，cleanup refs 依赖局部化到 cleanup/report 路径。
192. Runtime Content Verification Sub-Lifecycle Facade Dependency Cleanup 已完成第一版：`RuntimeContentVerificationLifecycle.h` 已确认只保留 forward declaration，implementation 不再依赖 generic `RuntimeVerificationLifecycle` facade，而是直接委托 `RuntimeVerificationStartupProfileLifecycle` 与 `RuntimeVerificationPreparedSceneLifecycle`。
193. Runtime Application Shutdown Verification Bridge Cleanup Lifecycle Direct Dependency Cleanup 已完成第一版：`RuntimeApplicationShutdownVerificationBridge.cpp` 不再依赖 generic `RuntimeVerificationLifecycle` facade，而是直接委托 `RuntimeVerificationCleanupLifecycle` 输出 renderer subsystem cleanup 与 Engine cleanup verification report；shutdown bridge public API 与 cleanup order 不变。
194. Runtime Verification Lifecycle Facade Removal 已完成第一版：`RuntimeFrameLifecycle.cpp` 不再依赖 generic `RuntimeVerificationLifecycle` facade，而是直接调用 `RuntimeVerificationStopPolicy` 与 `RuntimeVerificationFrameCaptureLifecycle`；无调用者的 `RuntimeVerificationLifecycle.h/.cpp` 已删除并从 VS project/filter 注册中移除。
195. Runtime Application Editor Startup State Parameter and Callback Binder Include Cleanup 已完成第一版：editor startup lifecycle 不再接收未使用的 `RuntimeApplicationState&`，startup 调用点改为 `initializeEditor(config, window)`，callback binder implementation 移除冗余完整 config/state include；实际 state accessor call sites 仍显式 include state header。
196. Runtime Application Config Include Surface Follow-up Cleanup 已完成第一版：`RuntimeApplicationFrameEditorCallbackBridge.cpp` 不再 include 完整 `RuntimeApplicationConfig.h`，`RuntimeApplicationRunner.h` 改为 forward declare `RuntimeApplicationShellConfig`，完整 config include 局部化到 runner implementation。
197. Runtime Verification Args Public Header Config Forward Boundary Cleanup 已完成第一版：`RuntimeVerificationArgs.h` 不再 include 完整 `RuntimeApplicationConfig.h`，只 forward declare `RuntimeApplicationShellConfig`；完整 config include 局部化到 args implementation 和 application entry 调用点。
198. Runtime PBR Verification Args Compatibility Facade Removal 已完成第一版：无外部引用的 `RuntimePBRVerificationArgs.h/.cpp` 兼容空壳已删除，并从 VS project/filter 注册中移除；runtime verification args 入口收敛到 `RuntimeVerificationArgs.h/.cpp`。
199. Runtime Camera/Scene Prepare Config Consumer Header Forward Boundary Cleanup 已完成第一版：`RuntimeCameraLifecycle`、`RuntimeLegacyExperimentLifecycle`、`RuntimeSceneSetupContextFactory` 与 `RuntimeSceneSetupPipelineLifecycle` public headers 只 forward declare camera/scene prepare config DTO，完整 DTO include 局部化到对应 implementation。
200. Runtime Engine Lifecycle State/Cleanup Refs Header Split 已完成第一版：旧 `RuntimeEngineLifecycleTypes.h` 拆为 state 与 cleanup refs 两个窄头，frame/state 路径和 shutdown cleanup/report 路径不再共享宽类型头。
201. Runtime Frame Lifecycle Types Compatibility Aggregator Removal 已完成第一版：无源码 include 的旧 frame lifecycle config/state 聚合头已删除，VS project/filter 注册同步移除，`RuntimeFrameLifecycleConfig.h` 与 `RuntimeFrameLifecycleState.h` 保持唯一 canonical 窄头入口。
202. Runtime Application Public Header Include Boundary Cleanup 已完成第一版：`RuntimeApplicationCallbackBinder.h` 只 forward declare `RuntimeBootstrapperCallbacks`，`RuntimeApplicationWindowStartupLifecycle.h` 只 forward declare `RuntimeWindowSnapshot`，完整 DTO 依赖局部化到实际构造或按值返回的 implementation。
203. Runtime PBR Verification Config Include Boundary Cleanup 已完成第一版：PBR profile facade 与 prepared-scene stats implementation 不再 include 完整 `RuntimeVerificationConfig.h`，完整 config 依赖保留在实际读取字段的 PBR verification 子模块。
204. Runtime Editor Lifecycle Callback DTO Header Boundary Cleanup 已完成第一版：`RuntimeEditorLifecycle.h` 只 forward declare `RuntimeFrameCallbacks`，完整 callback DTO 依赖局部化到 editor lifecycle implementation 与 frame editor callback bridge implementation。
205. Engine Lifecycle Snapshot Type Header Extraction 已完成第一版：`EngineLifecycleSnapshot.h` 承载 Engine lifecycle snapshot DTO，runtime engine verification report formatter 不再为了格式化快照字段 include 完整 `Engine.h`。
206. Engine Run Mode Header Extraction 已完成第一版：`EngineRunMode.h` 单独承载 run mode 枚举，`EngineLifecycleSnapshot.h` 不再为了 run mode include 完整 `EngineContext.h`。
207. Engine Desc Header Extraction 已完成第一版：`EngineDesc.h` 单独承载 Engine startup desc DTO，application config policy / engine startup bridge 不再为了构造启动 desc include 完整 `EngineContext.h`。
208. Runtime Application Shutdown Verification Config Boundary Cleanup 已完成第一版：shutdown verification bridge 改为接收 `RuntimeVerificationConfig`，不再为了 cleanup report include 完整 `RuntimeApplicationConfig.h`。
209. Engine Public Header Context Ownership Boundary Cleanup 已完成第一版：`Engine.h` 不再 public include `EngineContext.h` / `World.h`，`EngineContext` 改由 `Engine.cpp` 通过 `std::unique_ptr` 完整拥有并初始化。
210. Engine AddSubsystem Context Helper Boundary Cleanup 已完成第一版：`Engine::addSubsystem(...)` public template 不再直接解引用 `mContext`，initialized-subsystem context handoff 下沉到 `Engine.cpp` 私有 helper。
211. Runtime Application Shutdown Cleanup Verification Config Boundary Cleanup 已完成第一版：shutdown cleanup bridge 改为接收 `RuntimeVerificationConfig`，完整 shell config 到 verification config 的映射集中到 shutdown lifecycle facade。
212. Runtime Application Shutdown Lifecycle Verification Config Boundary Cleanup 已完成第一版：shutdown lifecycle facade 改为接收 `RuntimeVerificationConfig`，完整 shell config 到 verification config 的映射上移到 callback binder。
213. Runtime Application Callback Binder Shutdown Bridge Boundary Cleanup 已完成第一版：新增 shutdown callback bridge，callback binder 不再直接 include shutdown lifecycle 或 config policy，shutdown callback 的 verification config 映射集中到 bridge implementation。
214. Runtime Application Callback Binder Frame Bridge Boundary Cleanup 已完成第一版：新增 frame callback bridge，callback binder 不再直接 include frame lifecycle，frame shouldContinue/runFrame 转发集中到 bridge implementation。
215. Runtime Application Callback Binder Startup Bridge Boundary Cleanup 已完成第一版：新增 startup callback bridge，callback binder 不再直接 include startup lifecycle，startup initialize 转发集中到 bridge implementation。
216. Runtime Bootstrapper Callbacks Header Extraction 已完成第一版：新增 `RuntimeBootstrapperCallbacks.h` 承载 callback DTO，`RuntimeBootstrapper.h` 只保留 runner facade 与 DTO 前置声明；callback binder / shell 只依赖 DTO 头，runner 同时显式依赖 runner facade 与 DTO。
217. Runtime Frame Callback Default Argument Header Boundary Cleanup 已完成第一版：`RuntimeFrameRunner.h` 与 `RuntimeFrameLifecycle.h` 用无 callback overload 替代 `RuntimeFrameCallbacks` 默认参数，public headers 只 forward declare callback DTO；完整 callback include 局部化到 runner/lifecycle implementation 和实际构造 editor frame callbacks 的 frame run bridge。
218. Runtime Application Shell Config Header Boundary Cleanup 已完成第一版：`RuntimeApplicationShell.h` 不再 include 完整 `RuntimeApplicationConfig.h`，shell config 改由 private owning pointer 持有；完整 config 依赖局部化到 shell implementation 和 runner composition root。
219. Engine World Persistent Level Header Boundary Cleanup 已完成第一版：`World.h` 不再 include 完整 `Level.h`，persistent level 通过 forward declaration + out-of-line destructor 隐藏；实际构造或遍历 Level 的实现文件显式 include `Level.h`。
220. 下一步建议继续 Engine public header 低风险 implementation detail audit，或回到 callback/bootstrapper include surface audit。renderer 侧只推进通用 backend contract，不扩张 PBR pass。

## 约束

- 不引入 UE 的宏系统。
- 不实现 UObject GC。
- 不实现 reflection。
- 不实现蓝图。
- 不让 renderer 再决定引擎框架。
