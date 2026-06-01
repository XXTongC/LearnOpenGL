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
220. Engine Actor Root SceneComponent Header Boundary Cleanup 已完成第一版：`Actor.h` 不再 include 完整 `SceneComponent.h`，root component pointer API 改由 forward declaration 暴露；`Actor.cpp` 显式 include `SceneComponent.h` 以支持 register/dynamic_cast 逻辑。
221. Engine AssetSubsystem Registry Header Boundary Cleanup 已完成第一版：`AssetSubsystem.h` 不再 include 完整 `AssetRegistry.h`，registry 通过 private owning pointer 隐藏；实际访问 registry API 的实现文件显式 include `AssetRegistry.h`。
222. Renderer Backend Frame Types Header Extraction 已完成第一版：新增 `RendererBackendFrameTypes.h` 承载 `RendererFrameIntent` / `RendererFrameResult`，frame bridge public headers 不再为了 frame DTO 或 backend slot snapshot 传递完整 `RendererBackend.h` / `RendererSubsystemBackendSlot.h`。
223. RendererSubsystem Backend Slot Header Boundary Cleanup 已完成第一版：`RendererSubsystemBackendSlot.h` 不再 include 完整 `RendererBackend.h`，backend owner 通过 forward declaration + out-of-line destructor 隐藏；实际调用 backend virtual API 的 implementation 显式 include `RendererBackend.h`。
224. Runtime Renderer Backend Catalog Registry Header Boundary Cleanup 已完成第一版：`RuntimeRendererBackendCatalog.h` 不再 include 完整 `RendererBackendRegistry.h`，catalog public surface 只传播 registry DTO/types 与 registry forward declaration；完整 registry 构造和查询依赖局部化到 catalog implementation。
225. Runtime Renderer Backend Catalog Registry Object API Cleanup 已完成第一版：`RuntimeRendererBackendCatalog.h` 不再公开返回具体 `RendererBackendRegistry` object 的 `makeRegistry()` API；registry 构造收敛为 catalog implementation-local helper，public facade 只保留轻量 key/query/selection/attachment API。
226. Engine Level Actor Header Boundary Cleanup 已完成第一版：`Level.h` 不再 include 完整 `Actor.h`，actor owner 列表通过 forward declaration + out-of-line destructor 隐藏；实际遍历/生命周期调用 actor 的 implementation 显式 include `Actor.h`，`spawnActor<T>` 调用点继续由具体 actor 类型 include 保障。
227. Engine Legacy Scene Transform Header Boundary Cleanup 已完成第一版：`LegacySceneWorldBuilder.h` 与 `WorldLegacySceneExporter.h` 不再 include 完整 `Transform.h`，Transform 只通过 forward declaration 暴露；实际读取/写入 transform 字段的 import/export implementation 显式 include `Transform.h`。
228. Engine ScenePackage Load Result World Owner Boundary Cleanup 已完成第一版：`ScenePackageLoadResult` 的 `std::unique_ptr<World>` 特殊成员改为 out-of-line default，完整 `World.h` 依赖保持在 `ScenePackage.cpp`，load result 继续保持 move-only 返回语义。
229. Renderer Backend Contract Frame DTO Header Boundary Cleanup 已完成第一版：`RendererBackend.h` 不再 include 完整 `RendererBackendFrameTypes.h`，backend contract 只 forward declare `RendererFrameIntent` / `RendererFrameResult`；实际读取 frame intent 或构造 frame result 的 runtime backend implementation 显式 include DTO 头。
230. Runtime Frame Pipeline Context Header Boundary Cleanup 已完成第一版：`RuntimeFramePipeline.h` 与 `RuntimeFramePasses.h` 不再 include 完整 `AppRuntimeContext.h`，frame pipeline/pass public headers 只保留 runtime context/config forward declarations；实际读取 context/config 字段的 implementation 显式 include 完整头。
231. Runtime Frame Pass Registry Key String View Boundary Cleanup 已完成第一版：`RuntimeFramePassRegistry.h` 的 pass key lookup 改为 `std::string_view`，registry public header 不再为了只读 key 查询 include `<string>`；trim/token 字符串处理保留在 implementation。
232. Engine Lifecycle Snapshot Header Boundary Cleanup 已完成第一版：`Engine.h` 不再 include 完整 `EngineLifecycleSnapshot.h`，只 forward declare `EngineLifecycleSnapshot`；实际构造或读取快照字段的 `Engine.cpp`、Engine diagnostics panel 和 runtime verification report 显式 include 完整快照头。
233. Frame Render Targets Framebuffer Header Boundary Cleanup 已完成第一版：`FrameRenderTargets.h` 不再 include 完整 `framebuffer/framebuffer.h`，只 forward declare `Framebuffer` / `Texture`；实际 FBO 创建、FBO id 查询和 attachment 访问集中到 `FrameRenderTargets.cpp`。
234. PostProcess Pass Header Boundary Cleanup 已完成第一版：`PostProcessPass.h` 不再 include 完整 framebuffer/mesh/shader/settings headers，只 forward declare 引用参数类型；post-process resolve/composite/bloom 执行依赖集中到 `PostProcessPass.cpp`。
235. Bloom Header Framebuffer Boundary Cleanup 已完成第一版：`Bloom.h` 不再 include 完整 core/framebuffer/geometry/shader headers，只 forward declare `Framebuffer` / `Texture` / `Shader` / `Geometry`；Bloom FBO 创建、texture binding 和 shader/quad 操作集中到 `Bloom.cpp`。
236. Environment Texture Header Boundary Cleanup 已完成第一版：`EnvironmentRenderTargets.h` 与 `EnvironmentProfile.h` 不再 include 完整 `framework/texture.h`，只 forward declare `Texture`；环境贴图创建、HDR/procedural texture 创建、IBL debug/precompute 和 PBR IBL/deferred lighting 中实际解引用 texture 的 implementation 显式 include 完整 texture 头。
237. IBL Precompute Pass Header Boundary Cleanup 已完成第一版：`IBLPrecomputePass.h` 不再 include 完整 environment targets、texture、mesh、shader library headers，也不再通过 private helper 暴露 `glm::mat4`；capture projection/view helper 和完整 IBL precompute 执行依赖集中到 `IBLPrecomputePass.cpp`。
238. Light Resource Binder Header Boundary Cleanup 已完成第一版：`LightResourceBinder.h` 不再 include 完整 `framework/shader.h` 或 light implementation headers，只 forward declare shader/light 参数类型；实际 uniform 写入和 light 字段读取依赖集中到 `LightResourceBinder.cpp`。
239. Depth Prepass Binder Header Boundary Cleanup 已完成第一版：`DepthPrepassBinder.h` 不再 include 完整 `framework/shader.h`、`mesh/mesh.h` 或 `MaterialBindingContext.h`，只 forward declare shader、mesh 和 material binding context 参数类型；实际 depth frame/object uniform 绑定依赖集中到 `DepthPrepassBinder.cpp`。
240. Material Binder Header Boundary Cleanup 已完成第一版：`MaterialBinder.h` 不再 include 完整 `framework/shader.h`、`materials/material.h`、`mesh/mesh.h` 或 `MaterialBindingContext.h`，只 forward declare shader、material、mesh 和 material binding context 参数类型；实际材质分发、texture binding、shader uniform 和 mesh 访问依赖集中到 `MaterialBinder.cpp`。
241. PBR Material Binder Header Boundary Cleanup 已完成第一版：`PBRMaterialBinder.h` 不再 include 完整 `framework/shader.h`、`PBRMaterial.h`、`mesh/mesh.h` 或 `MaterialBindingContext.h`，只 forward declare shader、PBR material、mesh 和 material binding context 参数类型；实际 PBR object/light/shadow/surface/IBL binder 编排依赖集中到 `PBRMaterialBinder.cpp`。
242. PBR Object Uniform Binder Header Boundary Cleanup 已完成第一版：`PBRObjectUniformBinder.h` 不再 include 完整 `framework/shader.h`、`PBRMaterial.h`、`mesh/mesh.h` 或 `MaterialBindingContext.h`，只 forward declare shader、PBR material、mesh 和 material binding context 参数类型；实际 model/view/projection/normal matrix、camera position、opacity/time uniform 写入依赖集中到 `PBRObjectUniformBinder.cpp`。
243. PBR Shadow Resource Binder Header Boundary Cleanup 已完成第一版：`PBRShadowResourceBinder.h` 不再 include 完整 `framework/shader.h` 或 `MaterialBindingContext.h`，只 forward declare shader 和 material binding context 参数类型；实际 CSM shadow、PBR shadow atlas、point shadow atlas、camera 和 shader uniform 依赖集中到 `PBRShadowResourceBinder.cpp`。
244. PBR Surface Resource Binder Header Boundary Cleanup 已完成第一版：`PBRSurfaceResourceBinder.h` 不再 include 完整 `framework/shader.h` 或 `PBRMaterial.h`，只 forward declare shader 和 PBR material 参数类型；实际 surface uniform、texture slot、texture binding 和 shader/material 依赖集中到 `PBRSurfaceResourceBinder.cpp`。
245. PBR IBL Resource Binder Header Boundary Cleanup 已完成第一版：`PBRIBLResourceBinder.h` 不再 include 完整 `framework/shader.h` 或 `PBRMaterial.h`，只 forward declare shader、PBR material 和 environment targets 参数类型；实际 IBL readiness 判断、IBL float slot、environment target 和 texture binding 依赖集中到 `PBRIBLResourceBinder.cpp`。
246. Shadow Resource Binder Header Boundary Cleanup 已完成第一版：`ShadowResourceBinder.h` 不再 include 完整 `camera/camera.h`、`framework/shader.h`、`directionalLight.h` 或 `pointLight.h`，只 forward declare 全局 `Camera` 以及 shader/light 参数类型；实际 CSM、point shadow、fallback directional shadow、shader uniform 和 light/camera 字段读取依赖集中到 `ShadowResourceBinder.cpp`。
247. PBR Alpha Shadow Binder Header Boundary Cleanup 已完成第一版：`PBRAlphaShadowBinder.h` 不再 include 完整 `framework/shader.h` 或 `mesh/mesh.h`，只 forward declare shader 和 mesh 参数类型，并显式 include glm 类型头；实际 alpha-masked PBR mesh 判断、alpha cutoff/albedo map shadow uniform 写入、mesh/material/texture 访问依赖集中到 `PBRAlphaShadowBinder.cpp`。
248. Shadow Render Pass Header Boundary Cleanup 已完成第一版：`DirectionalShadowRenderPass.h` 与 `PointShadowRenderPass.h` 不再 include 完整 camera/light/mesh/shader-library headers，只 forward declare `Camera`、light、mesh 和 shader library 参数类型；实际 shadow framebuffer、camera/light 字段、mesh draw、shader uniform 和 alpha-shadow 分支依赖集中到对应 `.cpp`。
249. Draw Helper Debug Quad Header Boundary Cleanup 已完成第一版：`MeshDraw.h`、`ShadowMeshDraw.h` 和 IBL/GBuffer/tiled/clustered debug quad pass headers 不再 include 完整 `mesh/mesh.h` 或 `materials/material.h`，只 forward declare mesh 参数/成员类型；实际 indexed mesh draw、screen quad 构造、material type 判断和 GL draw 调用依赖集中到对应 `.cpp`。
250. PBR Draw Pass Header Boundary Cleanup 已完成第一版：`PBRDepthPrepass.h`、`PBRGBufferPass.h` 与 `PBRSceneRenderPass.h` 不再 include 完整 mesh、material binding context 或 shader library headers，只 forward declare mesh/context/shader 参数类型；实际 PBR material 判断、depth/G-buffer/PBR forward shader binding、G-buffer target 和 mesh draw 依赖集中到对应 `.cpp`。
251. Scene Render Pass Header Boundary Cleanup 已完成第一版：`SceneRenderPass.h` 不再 include 完整 material、mesh、material binding context、shader 或 shader library headers，只 forward declare legacy scene draw pass 参数类型；实际 material 选择、render state、shader binding 和 mesh draw 依赖集中到 `SceneRenderPass.cpp`。
252. PBR Shadow Atlas Render Pass Header Boundary Cleanup 已完成第一版：`PBRShadowAtlasRenderPass.h` 不再 include 完整 camera、directional/point light、mesh 或 shader library headers，只保留 atlas stats/render-target 类型头和参数前置声明；实际 CSM cascade、point shadow cubemap face、alpha-mask shadow shader、mesh draw 和 shader uniform 依赖集中到 `PBRShadowAtlasRenderPass.cpp`。
253. PBR Deferred Lighting Grid Header Boundary Cleanup 已完成第一版：`PBRDeferredLightingPass.h`、`PBRDeferredLightBuffer.h`、`PBRDeferredTiledLightGrid.h` 与 `PBRDeferredClusteredLightGrid.h` 不再 include 完整 material binding context、mesh 或 GL core headers，只保留必要 stats/config/value-member 类型、`glm::ivec4` 轻量头和参数前置声明；实际 context 字段读取、light packing、CPU tiled/clustered grid 构建、GPU clustered dispatch、lighting quad mesh 和 GL buffer 操作依赖集中到对应 `.cpp`。
254. Renderer Infrastructure and Runtime Input Header Boundary Cleanup 已完成第一版：`RenderQueue.h`、`ShadowRenderer.h`、`ShaderLibrary.h`、`PBRShadowAtlasRenderTargets.h` 与 `RuntimeInputController.h` 不再传播 camera/scene/mesh/shader/core/camera-control 等 implementation-only headers；实际 render queue projection/sort、shadow renderer pass dispatch、shader construction、shadow atlas GL texture allocation 和 input controller camera/control 操作依赖集中到对应 `.cpp`。
255. Renderer Facade PImpl Header Boundary Cleanup 已完成第一版：`renderer.h` 不再按私有成员传播 render pass、queue、shadow renderer、shader library、render target、scene/camera/light/mesh/shader/core 等 implementation-only headers；`Renderer` 的内部渲染状态迁入 `Renderer::Impl` 并由 `renderer.cpp` 完整拥有，实际读取 stats/profile/environment/scene 的调用点改为显式 include 所需窄头。
256. SceneSetup Context Header Boundary Cleanup 已完成第一版：`SceneSetup.h` 不再传播 scene/light/material/mesh/Bloom/environment/frame-target/renderer/profile 完整 headers，只保留 `SetupContext` 引用/shared_ptr 契约和前置声明；实际 scene setup 构造、pipeline context 字段读取和 runtime lifecycle 按值持有完整 context 的依赖集中到对应 `.cpp`。
257. Editor Panels Public Header Boundary Cleanup 已完成第一版：`EditorPanels.h` 不再传播完整 camera/light/shadow/object/scene headers，只保留 selection/context DTO、edit transaction log、`glm` 值类型和前置声明；实际 hierarchy/inspector/selection 绘制依赖集中到 `EditorPanels.cpp`，`Scene -> Object` shared_ptr 转换依赖显式保留在 runtime editor panel coordinator implementation。
258. Runtime Editor Panel Coordinator Header Boundary Cleanup 已完成第一版：`RuntimeEditorPanelCoordinator.h` 不再传播完整 `AppRuntimeContext`、debug controller panel 或 editor panels headers，只 forward declare facade 参数/返回类型；完整 runtime context 字段读取、debug/editor panel context 构造和 draw function 调用依赖集中到 coordinator implementation。
259. Editor Selection State Header Extraction 已完成第一版：新增 `EditorSelectionState.h` 承载 selection context 与 edit transaction log，`RuntimeEditorLifecycleState.h` 不再为了持有 editor state include 完整 `EditorPanels.h`；`EditorPanels.h` 收敛为 panel context/draw facade，selection helper 声明随 state 窄头提供。
260. Editor Diagnostics Context Header Extraction 已完成第一版：新增 `DebugControllerContext.h` 与 `EngineDiagnosticsContext.h` 承载 debug/diagnostics DTO；`DebugControllerPanel.h` 与 `EngineDiagnosticsPanel.h` 收敛为 draw facade，只 forward declare context，实际构造/字段读取依赖集中到 implementation 和 coordinator。
261. Runtime Editor Lifecycle State Owner Boundary Cleanup 已完成第一版：`RuntimeEditorLifecycleState.h` 不再 include `EditorSelectionState.h` 或暴露 selection/transaction 字段；完整 `SelectionContext` 与 `EditTransactionLog` 由 `RuntimeEditorLifecycleState.cpp` 通过 PImpl 拥有，runtime editor lifecycle 通过访问器取得引用。
262. Runtime Viewport Header Boundary Cleanup 已完成第一版：`RuntimeViewport.h` 不再 include 完整 camera、screen material 或 frame render targets headers，只 forward declare resize/viewport facade 参数类型；实际 camera aspect、post-process texture sync 和 GLFW framebuffer size 依赖集中到实现文件使用点。
263. Runtime Window Lifecycle Header Boundary Cleanup 已完成第一版：`RuntimeWindowLifecycle.h` 不再 include `RuntimeWindowLifecycleTypes.h`，只 forward declare window lifecycle DTO；实际 window callback context、snapshot 构造和 frame run/window startup DTO 字段读取依赖集中到 implementation 使用点。
264. Runtime GUI Host Types Header Extraction 已完成第一版：新增 `RuntimeGuiHostTypes.h` 承载 GUI init/frame context；`RuntimeGuiHost.h` 收敛为 host facade，不再传播 `<functional>` 或 DTO 字段，实际 GUI context 构造/读取依赖集中到 editor lifecycle 和 GUI host implementation。
265. Runtime Graphics Lifecycle Types Header Extraction 已完成第一版：新增 `RuntimeGraphicsLifecycleTypes.h` 承载 graphics lifecycle config；`RuntimeGraphicsLifecycle.h` 收敛为 graphics facade，只 forward declare config，实际 config 构造/字段读取依赖集中到 config policy、graphics startup 和 lifecycle implementation。
266. Runtime Frame Lifecycle State Owner Boundary Cleanup 已完成第一版：`RuntimeFrameLifecycleState.h` 不再 include `RuntimeFrameClock.h` 或暴露 frame clock/count/capture fields；完整 state layout 由 `RuntimeFrameLifecycleState.cpp` 通过 PImpl 拥有，frame lifecycle implementation 通过访问器 reset/tick/count/capture。
267. Runtime Application Config Backend Key Default Boundary Cleanup 已完成第一版：`RuntimeApplicationConfig.h` 不再 include `RuntimeRendererBackendKeys.h`；默认 renderer backend key 由 `RuntimeApplicationConfig.cpp` 的 shell config 默认构造函数设置，backend key helper 依赖局部化到 config implementation 和实际 backend selection/verification override 使用点。
268. Engine Actor Component Header Boundary Cleanup 已完成第一版：`Actor.h` 不再 include 完整 `ActorComponent.h`，只 forward declare component 指针/owner 类型；`Actor` 析构迁入 `Actor.cpp`，完整 component lifecycle API 依赖局部化到 Actor implementation 和实际 inspector/snapshot/export 调用点。
269. Engine Subsystem Public Header Boundary Cleanup 已完成第一版：`Engine.h` 不再 include 完整 `EngineSubsystem.h`，只 forward declare subsystem owner 类型；完整 subsystem initialize/tick/shutdown/diagnostics API 依赖局部化到 `Engine.cpp` 和具体 subsystem 派生类 headers。
270. Legacy Scene World Stats Header Extraction 已完成第一版：新增 `LegacySceneWorldStats.h` 承载 legacy import/export stats DTO；`SceneSetupPipeline.h` 与 `WorldDrivenSceneSetup.h` 不再为了 result stats 传播完整 legacy builder/exporter 行为头。
271. World Driven Scene Stats Header Extraction 已完成第一版：新增 `WorldDrivenSceneStats.h` 承载 world-driven scene probe/minimal scene stats DTO；`SceneSetupPipeline.h` 不再为了 result stats 传播完整 `WorldDrivenSceneSetup.h` 行为头，formatter 与 add scene 行为依赖局部化到 implementation。
272. PBR Light Rig Profile Header Boundary Cleanup 已完成第一版：`PBRLightRigProfile.h` 不再 include 完整 light 行为头，只保留 `glm` 值类型和 light 参数前置声明；实际 ambient/directional/spot/point light 创建、字段读写和 point shadow count 更新依赖局部化到 `PBRLightRigProfile.cpp`。
273. PBR Experiment Profile Header Boundary Cleanup 已完成第一版：`PBRExperimentProfile.h` 不再 include 完整 environment/postprocess/preview/light/camera profile 头，只保留 storage facade 和引用参数前置声明；实际 profile 复制、PropertyBuilder 构造和 config 读写依赖局部化到 `PBRExperimentProfile.cpp`。
274. PBR Material Profile Header Extraction 已完成第一版：新增 `PBRMaterialProfile.h` 承载 material profile/storage 窄接口；`PBRPreviewProfile.h` 不再为了 by-value material preset 传播完整 `PBRMaterial.h` runtime material 行为头。
275. Assimp Loader Public Header Boundary Cleanup 已完成第一版：`AssimpMaterialImporter.h`、`assimpLoader.h` 与 `assimpInstanceLoader.h` 不再传播 full material/Assimp/mesh/renderer/texture/shader implementation headers；import helper 细节局部化到 `.cpp`。
276. Material Types Header Extraction 已完成第一版：新增 `MaterialTypes.h` 承载 `MaterialType` / `PreStencilType`；`ShaderLibrary.h` 不再为了 shader map key 传播完整 `material.h`。
277. Legacy Experiment Runner Implementation Split 已完成第一版：`LegacyExperimentRunner` 不再是 header-only，历史实验构建/更新逻辑迁入 `.cpp`，public header 只保留 `RuntimeContext`、实验 API 声明和类型前置声明。
278. Legacy Experiment Runner Private State PIMPL Cleanup 已完成第一版：`LegacyExperimentRunner.h` 不再暴露 solar system / orbiting point light 私有状态布局、历史实验 enable flags 或 `Object` 依赖，完整历史实验状态由 `.cpp` 中的 `Impl` 拥有。
279. Runtime Frame Clock Private State PIMPL Cleanup 已完成第一版：`RuntimeFrameClock.h` 不再传播 `<chrono>` 或 `RuntimeFrameClockTypes.h`，完整 steady-clock time point 与 tick state 由 `.cpp` 中的 `Impl` 拥有。
280. Application Header Boundary Cleanup 已完成第一版：`Application.h` 不再传播 `texture.h`、`Logger.h` 或未使用的 `extern Logger logger`，窗口/application facade 只保留 callback、window 和 lifecycle API。
281. Assimp Instance Loader GLM Header Boundary Cleanup 已完成第一版：`assimpInstanceLoader.h` 不再传播完整 `glm.hpp`，只保留 `glm/fwd.hpp` 与 `const glm::mat4&` 参数契约；完整矩阵定义和 instanced matrix 写入依赖局部化到 implementation。
282. Runtime Profile State Storage Path Boundary Cleanup 已完成第一版：`RuntimeProfileState.h` 不再为了 default path 初始化传播 `RendererFramePassProfile.h` 或 `PBRExperimentProfile.h`；storage-only 依赖和 path 初始化局部化到 `RuntimeProfileState.cpp`，profile loader 显式 include 真实 storage 使用点。
283. Runtime Render Resource PostProcessPass Owner Boundary Cleanup 已完成第一版：`RuntimeRenderResourceState.h` 不再传播 `PostProcessPass.h`，post-process pass 由 implementation-owned pointer 持有，frame pass implementation 显式 include 并通过访问器执行 resolve/bloom/composite。
284. Runtime Render Resource FrameRenderTargets Owner Boundary Cleanup 已完成第一版：`RuntimeRenderResourceState.h` 不再传播 `FrameRenderTargets.h`，frame render targets 由 implementation-owned pointer 持有，runtime frame pass、backend readiness、scene setup 和 resize callback 路径通过访问器取得引用。
285. Runtime Profile State Camera Rig Owner Boundary Cleanup 已完成第一版：`RuntimeProfileState.h` 不再传播 `PBRCameraRigProfile.h`，camera rig profile 由 implementation-owned pointer 持有，profile loader、PBR light/camera verification 和 debug controller context 构造通过访问器取得引用。
286. Runtime Profile State Environment Profile Owner Boundary Cleanup 已完成第一版：`RuntimeProfileState.h` 不再传播 `EnvironmentProfile.h`，environment profile 由 implementation-owned pointer 持有，startup verification、profile loader、scene setup 和 debug controller context 构造通过访问器取得引用。
287. Runtime Profile State PostProcess Settings Owner Boundary Cleanup 已完成第一版：`RuntimeProfileState.h` 不再传播 `PostProcessSettings.h`，post-process settings 由 implementation-owned pointer 持有，profile loader、startup verification、runtime frame pass、renderer backend readiness 和 debug controller context 构造通过访问器取得引用。
288. Runtime Profile State Light Rig Owner Boundary Cleanup 已完成第一版：`RuntimeProfileState.h` 不再传播 `PBRLightRigProfile.h`，light rig profile 由 implementation-owned pointer 持有，PBR light/camera verification、profile loader、scene setup 和 debug controller context 构造通过访问器取得引用。
289. Runtime Profile State Preview Profile Owner Boundary Cleanup 已完成第一版：`RuntimeProfileState.h` 不再传播 `PBRPreviewProfile.h`，preview profile 由 implementation-owned pointer 持有，PBR preview verification、profile loader、scene setup 和 debug controller context 构造通过访问器取得引用。
290. Runtime Profile State Frame Pipeline Profile Owner Boundary Cleanup 已完成第一版：`RuntimeProfileState.h` 不再传播 `RuntimeFramePipelineProfile.h`，frame pipeline profile 由 implementation-owned pointer 持有，startup verification、profile loader、frame pass registry、frame pipeline、renderer backend frame plan key 和 debug controller context 构造通过访问器取得引用。
291. Runtime Render Resource Bloom Owner Boundary Cleanup 已完成第一版：`RuntimeRenderResourceState.h` 不再公开暴露 Bloom shared_ptr owner，Bloom owner 由 private `mBloom` 持有，scene setup 注入、Bloom frame pass 和 backend readiness 通过访问器取得引用。
292. Runtime Render Resource Screen Quad Owner Boundary Cleanup 已完成第一版：`RuntimeRenderResourceState.h` 不再公开暴露 screen quad shared_ptr owner，screen quad owner 由 private `mScreenQuad` 持有，scene setup 注入、screen composite pass 和 backend readiness 通过访问器取得引用。
293. Runtime Render Resource Screen Material Owner Boundary Cleanup 已完成第一版：`RuntimeRenderResourceState.h` 不再公开暴露 screen material shared_ptr owner，screen material owner 由 private `mScreenMaterial` 持有，scene setup 注入和 resize 后 post-process input texture sync 通过访问器取得引用。
294. Runtime Render Resource Scene Mesh/Material Owner Boundary Cleanup 已完成第一版：`RuntimeRenderResourceState.h` 不再公开暴露 `grassMaterial`、`skyBoxMesh`、`movePlane`、`textD`、`csmShadowMaterial` scene/legacy mesh/material owners，scene setup、legacy experiment 和 editor debug panel 注入通过访问器取得引用。
295. Runtime Render Resource Dead Point Light Mesh Owner Removal 已完成第一版：`RuntimeRenderResourceState.h` 删除无源码使用点的 `meshPointLight` 公开 owner 字段；未新增 replacement accessor。
296. Runtime Render Resource Renderer/Scene Owner Boundary Cleanup 已完成第一版：`RuntimeRenderResourceState.h` 不再公开暴露 `renderer`、`sceneOffScreen`、`sceneInScreen` shared_ptr owners，frame pass、scene setup、legacy experiment、editor context、renderer backend readiness 和 verification/report 路径通过访问器取得引用。
297. Runtime Render Resource Clear Color State Boundary Cleanup 已完成第一版：`RuntimeRenderResourceState.h` 不再公开暴露 `clearColor` 值型字段，frame runner 通过 `clearColor()` accessor 把当前 clear color 同步到 renderer。
298. Runtime Render Resource ReadOnly View Facade Cleanup 已完成第一版：新增 `RuntimeRenderResourceView` 与 `readOnlyView()`，renderer backend readiness、renderer backend attachment 和 verification report 的只读路径先迁到 read-only facade。
299. Runtime Render Resource ReadOnly View PBR Stats Consumer Cleanup 已完成第一版：PBR renderer stats 与 prepared scene stats 两个只读 verification collector 改为通过 `RuntimeRenderResourceView` 读取 renderer / scene，不再直接依赖 mutable render resource accessor。
300. Runtime Render Resource Renderer Pass Profile Access Boundary Cleanup 已完成第一版：`RuntimeRenderResourceState` 新增 `rendererFramePassProfile()`，PBR pass profile verification 与 runtime profile loader 不再直接取得 renderer owner 写 frame pass profile。
301. Runtime Render Resource Renderer Clear Color Sync Boundary Cleanup 已完成第一版：`RuntimeRenderResourceState` 新增 `syncClearColorToRenderer()`，frame runner 不再直接取得 renderer owner 只为同步 clear color。
302. Runtime Render Resource PBR Scene Probe Boundary Cleanup 已完成第一版：PBR scene probe verification 通过 `pbrMaterialShader()` 与 `addOffScreenSceneChild(...)` 取得 shader / 添加 probe，不再直接访问 renderer/scene owner。
303. Runtime Render Resource Imported Asset Probe Scene Boundary Cleanup 已完成第一版：imported asset probe verification 通过 `hasOffScreenSceneAndRenderer()` 与 `addOffScreenSceneChild(...)` 判断 readiness / 添加 probe，不再直接访问 offscreen scene owner；`AssimpLoader::loadPBR` 的 renderer 依赖保留为后续 asset-loading adapter 任务。
304. Runtime Asset Import Service Adapter Cleanup 已完成第一版：新增 `RuntimeAssetImportService` 隔离 Assimp PBR loader 对 renderer 的依赖，`RuntimeImportedAssetVerification` 不再直接访问 renderer/scene owner 或 `AssimpLoader`。
305. Runtime Engine World Verification ReadOnly Resource Cleanup 已完成第一版：Engine World verification 的 prepared scene stats 与 scene package round-trip resolver 创建改为通过 `RuntimeRenderResourceView` 读取 render resources，不再直接访问 mutable renderer/scene owner。
306. Runtime Frame Render Resource Adapter Cleanup 已完成第一版：新增 `RuntimeFrameRenderResourceAdapter` 集中 runtime frame pass execution 对 renderer/scene/frame-target/post-process 资源的访问，`RuntimeFramePasses` 不再直接访问 render resource owner。
307. Runtime Editor Render Resource Adapter Cleanup 已完成第一版：新增 `RuntimeEditorRenderResourceAdapter` 集中 editor panel/debug controller 对 renderer/text object/offscreen scene/inscreen scene/default selection scene 的访问，`RuntimeEditorPanelCoordinator` 不再直接访问 render resource owner。
308. Runtime Scene Setup Resource Adapter Cleanup 已完成第一版：新增 `RuntimeSceneSetupResourceAdapter` 集中 scene setup 与 legacy experiment DTO 构造对 renderer/scene/frame-target/Bloom/screen/legacy mesh-material 资源的访问，`RuntimeSceneSetupContextFactory` 与 `RuntimeLegacyExperimentLifecycle` 不再直接访问 render resource owner。
309. Runtime Window Resize Resource Adapter Cleanup 已完成第一版：新增 `RuntimeWindowRenderResourceAdapter` 集中 window resize callback 对 frame render targets / screen material 的访问，`RuntimeWindowLifecycle` 不再直接访问 resize render resource owner 或 `RuntimeViewport` implementation。
310. Runtime Renderer Backend Resource Adapter Cleanup 已完成第一版：新增 `RuntimeRendererBackendResourceAdapter` 集中 renderer backend attachment/report 对 runtime renderer pointer 的存在性检查、subsystem attachment 和 attachment comparison，content renderer backend lifecycle、attachment lifecycle 与 verification report 不再为了 renderer pointer 创建过宽 `RuntimeRenderResourceView`。
311. Runtime PBR Stats Resource Adapter Cleanup 已完成第一版：新增 `RuntimePBRStatsResourceAdapter` 集中 PBR renderer stats 与 prepared-scene stats 对 renderer / offscreen scene 的只读访问和 prepared scene traversal，PBR stats report 模块不再直接创建 `RuntimeRenderResourceView` 或依赖 scene/object/material/mesh/renderer implementation headers。
312. Runtime Frame Readiness Resource Adapter Cleanup 已完成第一版：新增 `RuntimeFrameReadinessResourceAdapter` 集中 runtime frame pass readiness 对 renderer / offscreen scene / frame render targets / Bloom / screen quad 的只读资源判断，`RuntimeRendererFrameBridgeAdapter` 不再直接创建 `RuntimeRenderResourceView` 或依赖 frame target / post-process implementation headers。
313. Runtime Engine World Verification Resource Adapter Cleanup 已完成第一版：新增 `RuntimeEngineWorldVerificationResourceAdapter` 集中 Engine World prepared-scene mesh stats traversal 与 runtime-generated scene package resolver load，`RuntimeEngineWorldVerification.cpp` 不再直接创建 `RuntimeRenderResourceView`、调用 `readOnlyView()`、读取 renderer/offscreen scene accessor 或持有 scene traversal/resolver implementation。
314. Runtime Probe Scene Resource Adapter Cleanup 已完成第一版：新增 `RuntimeProbeSceneResourceAdapter` 集中 verification probe 对 offscreen scene readiness、PBR probe geometry 创建和 probe object 注入的访问，`RuntimePBRSceneProbeVerification.cpp` 与 `RuntimeImportedAssetVerification.cpp` 不再直接调用 render resource probe helper，`RuntimeRenderResourceState` 删除 probe-only public helper。
315. Runtime Renderer State Resource Adapter Cleanup 已完成第一版：新增 `RuntimeRendererStateResourceAdapter` 集中 frame runner clear color sync 与 renderer frame pass profile access，`RuntimeFrameRunner.cpp`、`RuntimePBRPassProfileVerification.cpp` 与 `RuntimeProfileLoader.cpp` 不再直接调用 render resource state 窄 helper，`RuntimeRenderResourceState` 删除 `syncClearColorToRenderer()` 与 `rendererFramePassProfile()` public helper。
316. Runtime Render Resource ReadOnly View Facade Removal 已完成第一版：`RuntimeFrameReadinessResourceAdapter.cpp` 改为直接使用 `RuntimeRenderResourceState` const accessor，`RuntimeRenderResourceState` 删除 `RuntimeRenderResourceView` class 与 `readOnlyView()` public facade，application 源码不再存在 read-only view 过渡 API。
317. Runtime Frame Pass Registry Profile Predicate Cleanup 已完成第一版：`RuntimeFramePassRegistry` 的 pass enabled predicate 从完整 `AppRuntimeContext` 收窄到 `RuntimeFramePipelineProfile`，registry implementation 不再 include `AppRuntimeContext.h`，frame pipeline 与 renderer frame bridge readiness 共用 profile 引用判断 pass enabled。
318. Render resource decoupling 已达到阶段性收束条件：剩余 direct render resource accessor 已集中在 resource adapter/service implementation；下一步建议转入更高层 Engine runtime ownership、editor/gameplay boundary、场景/资产生命周期或系统化 UI/inspector 边界整理。
319. Runtime Inspector Implementation Split 已完成第一版：`PropertyInspector.h` 和 `MaterialInspector.h` 不再作为 header-only implementation 传播 ImGui、完整 material/texture 或 `PropertyInspector.h` 间接依赖，property/material inspector 绘制实现迁入 `tools/inspector/*.cpp` 并注册到 VS 工程。
320. 下一步建议继续推进系统化 UI/inspector：优先把 `EditorPanels.cpp` 内部的 Light / Shadow / Camera inspector 直写 ImGui 逻辑迁入 property schema builder 或独立 inspector facade，让对象声明属性、UI 统一生成。
321. Runtime Scene Object Inspector Schema Cleanup 已完成第一版：新增 `SceneObjectInspector` 集中 Light / Shadow / Camera 的 property schema 与 type name 判断，`EditorPanels.cpp` 不再直接写这些 inspector 的 ImGui 控件；`PropertySchema` 新增 `InputFloat` / `InputInt` 以保留原输入框控件语义。
322. 下一步建议继续迁出 `EditorPanels.cpp` 中 legacy object transform inspector 或 Actor/Component schema builder，让 selection panel 进一步收敛为选中对象分发、schema 绘制和 selection action。
323. Runtime Legacy Object Transform Inspector Schema Cleanup 已完成第一版：legacy object Position / Rotation / Scale 已迁入 `SceneObjectInspector::buildObjectTransformPropertySchema(...)`；`PropertySchema` 新增 `SliderVec3`，`EditorPanels.cpp` 不再直写 object transform 的 `InputFloat3` / `SliderFloat3` 控件。
324. 下一步建议继续迁出 `EditorPanels.cpp` 中 Actor/Component schema builder，或推进类型/组件 property provider 注册机制，让 selection inspector 从“按类型手写 schema”继续向系统化 UI 生成过渡。
325. Runtime Engine World Inspector Schema Extraction 已完成第一版：新增 `EngineWorldInspector` 集中 Actor / Component schema builder、SceneComponent transform edit、legacy Object transform sync、type/display name helper 与 undo helper；`EditorPanels.cpp` 只保留 Components tree、selection、transaction summary 和 snapshot action 编排。
326. 下一步建议继续把 Asset inspector schema 从 `EditorPanels.cpp` 迁出，或推进类型/组件 property provider 注册机制，让 selection panel 进一步收敛为目标分发与 action 编排。
327. Runtime Asset Inspector Schema Extraction 已完成第一版：新增 `AssetInspector` 集中 AssetDescriptor display name、imported-source 判断和 read-only property schema；`EditorPanels.cpp` 不再直写 Asset inspector 的 `PropertyBuilder`。
328. 下一步建议推进类型/组件 property provider 注册机制，或继续拆分 asset browser / hierarchy tree display helper，让 editor panels 更接近纯 selection/action shell。
329. Runtime Selection Inspector Panel Extraction 已完成第一版：新增 `SelectionInspectorPanel.cpp` 承载 selection target dispatch、Light / Shadow / Camera / Actor / Component / Asset inspector render helper 与 edit transaction summary；`EditorPanels.cpp` 进一步收敛为 hierarchy / asset browser / selection click shell。
330. 下一步建议继续拆分 hierarchy tree / asset browser tree display helper，或开始推进类型/组件 property provider 注册机制，让系统化 UI 从 facade schema 走向可注册的 inspector provider。
331. Runtime Hierarchy and Asset Browser Panel Extraction 已完成第一版：新增 `HierarchyPanel.cpp` 与 `AssetBrowserPanel.cpp` 承载两个 editor panel 的 tree/display helper；`EditorPanels.cpp` 只保留 selection state helper。
332. 下一步建议推进类型/组件 property provider 注册机制，或把 `EditorPanels.h` 继续拆为 panel context、selection API 与 panel facade 窄头，避免所有 editor panel implementation 共享宽 facade header。
333. Runtime Editor Panel Header Boundary Split 已完成第一版：新增 `EditorPanelContext.h` 与 `EditorPanelFacades.h`，`EditorPanels.h` 收敛为兼容聚合头；runtime coordinator、render resource adapter 与 panel implementation 改为按需 include 窄头。
334. 下一步建议推进类型/组件 property provider 注册机制，把当前按目标类型手写 dispatch 的 inspector facade 进一步收敛为可注册 provider。
335. Runtime Selection Inspector Provider Registry 已完成第一版：新增 `SelectionInspectorProviderRegistry`，selection inspector 的 Asset / Component / Actor / Shadow / Camera / Object 顶层目标分发改为默认 provider 注册与匹配。
336. 下一步建议继续把 provider factory 从 `SelectionInspectorPanel.cpp` 外移，或把 ActorComponent / Material 内部属性声明推进为可注册 property provider。
337. Runtime Selection Inspector Provider Factory Extraction 已完成第一版：新增 `SelectionInspectorProviders` 集中默认 provider 注册和绘制 helper，`SelectionInspectorPanel.cpp` 收敛为薄 panel shell。
338. 下一步建议把 ActorComponent 或 Material 的内部属性声明拆成可注册 property provider，让 provider registry 不只负责目标分发，也开始承载类型属性扩展点。
339. Runtime ActorComponent Property Provider Registry 已完成第一版：新增 `ActorComponentPropertyProviderRegistry` 与默认 `ActorComponentPropertyProviders`，Component 基础字段继续由 `EngineWorldInspector` 生成，SceneComponent / MeshComponent / LightComponent / CameraComponent / LegacyObjectComponent 专属字段改由 provider 追加。
340. 下一步建议继续把 Material inspector 或 Actor property schema 迁入类似 provider registry，让系统化 UI 扩展点覆盖更多运行时类型。
341. Runtime Actor Property Provider Registry 已完成第一版：新增 `ActorPropertyProviderRegistry` 与默认 `ActorPropertyProviders`，Actor 基础字段继续由 `EngineWorldInspector` 生成，Root SceneComponent section 改由 provider 追加。
342. 下一步建议继续把 Material inspector 接入 provider registry，先保留现有 material `visitEditableProperties(...)` 作为兼容 provider，再评估逐步迁出 material runtime 类中的 editor schema 依赖。
343. Runtime Material Property Provider Registry 已完成第一版：新增 `MaterialPropertyProviderRegistry` 与默认 `MaterialPropertyProviders`，`MaterialInspector` 改为通过默认 provider registry 构建 Material 属性；默认兼容 provider `legacy-visit-editable-properties` 仍委托现有 `Material::visitEditableProperties(...)`，保持 Material UI 行为不变。
344. 下一步建议逐步把具体 Material 类型字段从 runtime `visitEditableProperties(...)` 迁入 provider 模块，或先补一层 Material property DTO/accessor 边界，避免 editor schema 长期由 runtime material 类直接声明。
345. Runtime Material Property Provider Schema Extraction 已完成第一版：`MaterialPropertyProviderRegistry` 支持 `buildMatching(...)`，Material inspector schema 由通用 `render-state` provider 与具体 Material 类型 provider 叠加构建；`Material`、`PhongMaterial`、`PhongCSMShadowMaterial`、`PhongPointShadowMaterial`、`GrassInstanceMaterial`、`ScreenMaterial` 和 `PBRMaterial` 不再声明或实现 runtime inspector `visitEditableProperties(...)`。
346. 下一步建议继续把 Material provider 当前直接访问的 public material fields 收敛为 Material property DTO/accessor，或先拆分 `PBRMaterialProfile` config schema 与 editor `PropertyBuilder` 的边界。
347. Runtime Material Editable Accessor Boundary 已完成第一版：Phong / Grass / Screen / PBR provider 所需字段新增明确 edit accessors，`MaterialPropertyProviders.cpp` 不再直接访问这些 provider 所需 public material fields。
348. 下一步建议把本轮新增的零散 accessors 聚合为 Material property DTO，或逐步把 provider 已覆盖的 material public fields 下沉为 private，继续收缩 runtime material 数据布局暴露面。
349. Runtime Material Edit Controls DTO 已完成第一版：新增 `MaterialEditControls.h`，用 `PhongSurfaceEditControls`、`GrassMaterialEditControls`、`ScreenMaterialInputTextures` 和 `PBRMaterialEditControls` 聚合 provider 所需编辑入口；Material provider 不再调用上一轮一项字段一个函数的零散 accessors。
350. 下一步建议把 DTO 已覆盖的 material fields 逐步下沉为 private，或拆分 `PBRMaterialProfile` 配置 schema 与 editor `PropertyBuilder` 的依赖边界。
351. Screen Material Input Texture Encapsulation 已完成第一版：`ScreenMaterial` 新增 `setInputTextures(...)`，`mScreenTexture`、`mBloomTexture` 和 `mDepthStencilTexture` 下沉为 `private`；scene setup、resize 同步和 post-process composite 均改为通过明确 API / `ScreenMaterialInputTextures` DTO 访问。
352. 下一步建议继续按低风险 slice 私有化 Phong/Grass 的 surface texture 与 shininess 字段，或先为 PBRMaterial 增加完整 runtime setter/slot DTO 后再处理其大批公开字段。
353. Phong Surface Runtime State Encapsulation 已完成第一版：新增 `PhongSurfaceInput` / `PhongSurfaceRuntimeState`，`PhongMaterial`、`PhongPointShadowMaterial` 和 `PhongCSMShadowMaterial` 的 diffuse/specular/shininess 字段下沉为 `private`；renderer 通过 `surfaceState()` 读取，setup/import/legacy 路径通过 setter 或 `setSurface(...)` 写入。
354. 下一步建议继续私有化 `GrassInstanceMaterial` 的 surface/wind/cloud 字段，或先为 PBRMaterial 建立完整 runtime setter/slot DTO 后再处理 PBR 字段封装。
355. Grass Surface Runtime State Encapsulation 已完成第一版：新增 `GrassSurfaceInput` / `GrassSurfaceRuntimeState`，`GrassInstanceMaterial` 的 diffuse/specular/opacity/cloud/shininess 字段下沉为 `private`；renderer 通过 `surfaceState()` 读取，legacy grass field 与 instanced loader 通过 setter 写入。
356. 下一步建议转入 `PBRMaterial`，先建立完整 texture/surface/IBL/alpha/channel runtime setter/slot DTO，再分批私有化 PBR 的公开字段。
357. PBR Material Runtime State API 已完成第一版：新增 PBR surface/texture/channel/alpha/IBL input 与 runtime state DTO，`PBRMaterial` 暴露 grouped setter、单项 setter 和只读 state API；profile、renderer binder/pass 与 PBR stats 已迁入 API。
358. 下一步建议迁移 scene setup、importer、engine world probe 和 verification scene writer 到 `PBRMaterial` setter/API，然后按 surface、texture slot、channel、alpha、IBL 分批私有化 PBR 公开字段。
359. PBR Material Private Field Encapsulation 已完成第一版：scene setup、Assimp PBR importer、engine world scene setup/package resolver 与 PBR verification probes 已迁入 `PBRMaterial` setter/API；PBR texture、surface、channel、alpha mask 与 IBL 字段已下沉为 `private`。
360. 下一步建议继续收束 Material/profile 边界，优先拆分 `PBRMaterialProfile` config schema 对 editor `PropertyBuilder` 的依赖，或让 PBR profile/config 通过独立 provider/schema adapter 生成配置 UI。
361. PBR Material Profile Config Schema Adapter 已完成第一版：新增 `PBRMaterialProfileConfig` schema adapter 与 `PBRMaterialProfile.cpp`，`PBRMaterialProfile.h` 不再声明 `visitEditableProperties(PropertyBuilder&)`，`PBRMaterial.cpp` 不再承载 profile storage 或 editor/config schema。
362. 下一步建议继续处理仍直接在 runtime/profile 类型中暴露 `visitEditableProperties(PropertyBuilder&)` 的配置对象，例如 EnvironmentProfile、PostProcessSettings、RendererFramePassProfile 或 PBRPreviewProfile。
363. Post Process Settings Config Schema Adapter 已完成第一版：新增 `PostProcessSettingsConfig` schema adapter，`PostProcessSettings.h` 不再声明 `visitEditableProperties(PropertyBuilder&)`，DebugControllerPanel 和 PBR experiment preset 的 postprocess 子配置改为通过 adapter 构建 schema。
364. 下一步建议继续处理剩余直接暴露 `visitEditableProperties(PropertyBuilder&)` 的 profile/settings 对象，例如 EnvironmentProfile、RendererFramePassProfile、PBRPreviewProfile、PBRLightRigProfile、PBRCameraRigProfile 或 RuntimeFramePipelineProfile。

## 约束

- 不引入 UE 的宏系统。
- 不实现 UObject GC。
- 不实现 reflection。
- 不实现蓝图。
- 不让 renderer 再决定引擎框架。
