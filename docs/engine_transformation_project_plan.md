# UE5 启发式引擎化改造方案

## 方向修正

根据最新目标，项目不再按“当前只是 renderer，所以先补 renderer-facing engine boundary”的路线推进。新的路线是直接建立 UE5 启发式框架骨架，再把现有 renderer 迁入框架。

这意味着：

- `World / Level / Actor / Component` 成为第一主线。
- `EntityId / RenderWorldSnapshot` 暂时降级为未来内部实现细节。
- Renderer 不再作为架构中心。
- PBR 继续冻结为 renderer pipeline。

## 阶段计划

### Phase 1: Framework Skeleton

目标：

- 建立 `engine/` 目录。
- 实现 `EngineObject`、`Engine`、`World`、`Level`、`Actor`、`ActorComponent`、`SceneComponent`、`EngineSubsystem`。
- 注册工程并构建通过。

验收：

- 新框架代码独立编译。
- 不改变当前启动行为。
- 不修改 renderer 主流程。

### Phase 2: RendererSubsystem

目标：

- 新增 `RendererSubsystem`。
- 把现有 `GLframework::Renderer` 包装为 Engine subsystem。
- 先只桥接生命周期，不重写 renderer。

验收：

- 现有 renderer 仍能被旧 runtime 使用。
- 新 subsystem 能在 Engine 中 initialize / tick / shutdown。

### Phase 3: Actor Adapters

目标：

- 新增 `MeshActor` / `LightActor` / `CameraActor` 或对应 component adapter。
- 让旧 `Mesh`、`Light`、`Camera` 能挂到 Actor 框架下。

验收：

- 可以从 `World` 中枚举 Actor。
- 可以通过 Actor 找到旧渲染对象。

### Phase 4: World Driven Scene Setup

目标：

- 将当前 `SceneSetup` 的一部分迁移为生成 `World / Level / Actor`。
- 使用已完成的 `LegacySceneWorldBuilder` 作为过渡 bridge，先验证旧 `Scene / Object` 树可以被新框架表示。
- 使用已完成的 `WorldLegacySceneExporter` 作为反向 bridge，让新框架先生成场景结构，再通过 adapter 回填旧 renderer 所需 `Scene`。
- 使用已完成的 `WorldDrivenSceneSetup` 把 verification-only probe 抽成 `tools/sceneSetup` 下可复用的 World-driven scene setup helper。
- 使用已完成的 `SceneSetupPipeline` 作为 runtime scene prepare 的第一层可配置入口，默认仍调用旧 `prepareDefaultScene(...)`。
- `--verify-engine-world-scene-probe` 已改为通过 `SceneSetupPipelineConfig` 在 scene prepare 阶段追加 World-driven probe。
- `SceneSetup` 已拆出基础设施、旧默认内容和 PBR preview 三段，pipeline 可以复用 renderer / screen / environment / lights 而不强制创建旧默认场景。
- `--verify-engine-world-minimal-scene` 已完成真正的 World-driven minimal scene preset：由 `World / Level / Actor / Component` 创建 4 个 PBR mesh，再导出到旧 `Scene`，且不包含旧 PBR preview grid。
- `AppRuntimeContext` 已持有 `engineWorld`，World-driven scene setup 会填充 runtime World 后再导出给旧 renderer。
- `engineWorld` 已暴露给 Editor hierarchy，当前支持只读 `World -> Persistent Level -> Actor -> Component` 树显示和 Actor 选择。
- 默认 legacy scene path 已生成 read-only `engineWorld` mirror，普通启动也可以在 Editor 中看到新 World tree。
- 保留旧 scene path 作为 fallback。

验收：

- 一个最小 PBR probe 可以由 World setup 生成并进入旧 renderer。
- 一个最小 PBR scene 可以由 World setup 独立生成并进入旧 renderer。
- World-driven verification mode 可以证明 Actor 保留在 runtime World 中。
- 旧 verification 仍可通过。

### Phase 5: Editor Retargeting

目标：

- hierarchy 从旧 Object tree 转向 World / Level / Actor。当前已完成第一步：旧 tree 与新 `Engine World` tree 并行显示。
- inspector 从旧对象字段转向 Actor / Component。当前已完成 schema-driven Actor / Component 展示，并完成 World-driven scene 中 `SceneComponent` transform 的第一条可写提交路径。
- 当前默认 legacy scene path 已生成 read-only World mirror；legacy mirror 继续只读，World-driven scene 中具备旧 `Object` adapter 的 `SceneComponent` 可编辑并同步旧对象 transform。

验收：

- Editor 可以选择 Actor。已完成。
- ActorComponent 可以被 inspector 展示基础属性。已完成 schema-driven 展示，并已为 World-driven SceneComponent transform 开启受控编辑提交。
- 普通 legacy scene path 可以被 Editor hierarchy 以 World / Level / Actor 形式观察。已完成 read-only mirror。
- Component 节点可被直接选择，并进入专用 Component inspector。已完成第一版。
- 编辑提交进入最小 edit transaction boundary。已完成第一版，当前记录 target、field、before、after 和 dirty 状态。
- 支持撤销最近一条 `SceneComponent` Vec3 transform transaction。已完成最小 undo apply。
- Dirty/save boundary 已完成第一版：提供 `Mark Saved / Clear Transactions`，当前管理 dirty/history 状态，不执行磁盘保存。
- Scene transform snapshot persistence boundary 已完成第一版：World-driven scene 可保存 `SceneComponent` transform snapshot，并已加入 verification 断言。
- Scene transform snapshot load/apply boundary 已完成第一版：snapshot 可回放到 runtime `World`，并同步旧 renderer adapter；verification 会扰动组件后要求 apply 恢复。
- Stable scene path id 已完成第一版：snapshot 写入 actor/component stable path，apply 优先按 stable path 匹配，并已加入 verification 断言。
- Persistent id 字段已完成第一版：`EngineObject` 持有可序列化 persistent id，World-driven preset 为 `World / Level / Actor / SceneComponent` 写入确定性 id，snapshot apply 优先按 persistent id 匹配，并已加入 verification 断言。
- Legacy mirror persistent id coverage 已完成第一版：旧 `Scene / Object` 树导入 `engineWorld` 时会为 mirror World、Level、Actor 和 root SceneComponent 派生 deterministic persistent id，并已加入 verification 断言。
- Persistent id policy module 已完成第一版：`engine/PersistentIdPolicy` 集中定义 id source 和 `objectKind:source:scope:path` 格式，当前已接入 preset、legacy mirror、imported asset 与 editor-created。
- Imported asset persistent id coverage 已完成第一版：PBR import probe 会把 Assimp 导入对象树同步导入 runtime `engineWorld`，并以 `ImportedAssetDerived` 来源生成 Actor / SceneComponent persistent id。
- Editor-created persistent id coverage 已完成第一版：编辑器和 verification 共用 `EditorWorldActions::createEditorEmptyActor(...)`，新建 Actor / SceneComponent 使用 `EditorCreatedGenerated` 来源 id。
- Scene package save/load boundary 已完成第一版：`--verify-engine-world-scene-package` 证明最小 `World / Level / Actor / SceneComponent` 层级可以保存为 `engine.world.scenePackage.v1`，并加载成新的 runtime `World`，同时保留 persistent id、root component 和 parent attach。
- Scene package component type / adapter metadata 已完成第一版：package 会保存 actor/component type、adapter kind、runtime asset reference、object type 和 material type；load 阶段会恢复 typed component 类，并把尚未解析的 renderer references 计入 `unresolvedAdapterReferences`。
- Scene package asset resolver / renderer reconstruction 已完成第一版：`ScenePackageAssetResolver` 可以在 load 阶段解析 `runtime-generated:*` adapter reference，当前 verification 已把 unresolved references 压到 `0`，并证明 package-loaded World 可重新导出 mesh / light / legacy renderer object。
- AssetRegistry stable handle slice 已完成第一版：`engine/AssetRegistry` 定义稳定 `asset:<kind>:<source>:<path>` handle，scene package 写出 mesh / light / legacy-object `adapter.assetHandle` 并保留 `adapter.assetReference` fallback，verification 已断言 stable handle 全部 resolved。
- Scene package typed actor restore 已完成第一版：load 阶段恢复 `MeshActor`、`LightActor`、`LegacyObjectActor` 等动态 actor 类型，verification 已断言 `loadedTypedActors=6`。
- Scene package negative probes 已完成第一版：loader 会统计 unknown actor/component type 与 invalid transform，verification 已覆盖 missing schema、invalid line、missing count reject，以及 unknown type fallback。
- Imported asset stable handle coverage 已完成第一版：Assimp PBR import probe 会通过 `AssetRegistry` 注册 imported mesh/material/texture handles，verification 已断言 `meshAssetHandles=1`、`materialAssetHandles=1`、`textureAssetHandles=1`。
- Imported asset scene package manifest 已完成第一版：Assimp PBR import probe 会保存并加载 imported asset scene package，verification 已断言 mesh/material/texture manifest 持久化和读回。
- 下一步验收目标：把 imported asset handle 暴露到 editor asset browser / inspector 可见层；package 层可继续补 duplicate persistent id、self-parent/cycle attach、跨 actor parent reference 等 graph validation。

## 本轮执行

本轮直接开始 Phase 1，不再等待审核。

本轮交付：

- 文档方向修正。
- 第一批 `engine/` 骨架代码。
- 旧 `Scene / Object` 到 `World / Level / Actor` 的非侵入式导入 bridge。
- `World / Level / Actor` 到旧 `Scene` 的非侵入式导出 bridge 和 verification probe。
- `tools/sceneSetup` 下的 World-driven scene setup helper。
- `tools/sceneSetup` 下的 Scene setup pipeline 入口。
- `tools/sceneSetup` 下的 World-driven minimal scene preset。
- `AppRuntimeContext` 中的持久 `engineWorld` runtime 状态源。
- Assimp PBR import probe 到 runtime `engineWorld` 的 imported-asset persistent id bridge。
- 编辑器创建空 Actor 到 runtime `engineWorld` 的 editor-created persistent id bridge。
- 工程文件注册。
- 构建验证。
- `worked.md` 记录。
