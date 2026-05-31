# UE5 启发式 Phase 1 执行计划

## 状态

```text
Implemented and verified
```

## 本轮范围

实现最小 UE5 启发式 framework skeleton。

新增：

- `engine/Transform.h`
- `engine/EngineObject.h/.cpp`
- `engine/EngineContext.h`
- `engine/EngineSubsystem.h`
- `engine/ActorComponent.h/.cpp`
- `engine/SceneComponent.h/.cpp`
- `engine/Actor.h/.cpp`
- `engine/Level.h/.cpp`
- `engine/World.h/.cpp`
- `engine/Engine.h/.cpp`
- `engine/RendererSubsystem.h/.cpp`
- `engine/ActorAdapters.h/.cpp`
- `engine/LegacySceneWorldBuilder.h/.cpp`
- `engine/WorldLegacySceneExporter.h/.cpp`
- `tools/sceneSetup/WorldDrivenSceneSetup.h/.cpp`
- `tools/sceneSetup/SceneSetupPipeline.h/.cpp`

修改：

- `text2.vcxproj`
- `text2.vcxproj.filters`
- `work.md`
- `worked.md`

不做：

- 不改 `main.cpp`。
- 不接入 renderer。
- 不改旧 scene setup。
- 不改 PBR shader。
- 不提交 `imgui.ini`。

## 验证

本轮最低验证：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes showcase-spheres -DiscardCaptures
```

通过后才进入下一阶段。

当前验证结果：

```text
passed
```

验证时间：2026-05-25。

验证摘要：

- `Debug|x64` 构建通过。
- 新增 `engine/*.cpp` 已参与编译。
- `--verify-pbr-showcase-spheres` 运行通过。
- 当前 PBR showcase renderer stats 正常输出 `pbrShowcaseSpheres=6`、`pbrDeferredLightingDrawCalls=1`、`pbrDeferredTiledLightGridBound=yes`。
- `RendererSubsystem.cpp` 已参与编译，当前只作为 bridge，不接管 renderer 调度。
- `ActorAdapters.cpp` 已参与编译，当前提供旧 `Mesh / Light / Camera` 到 Actor/Component 的非侵入式桥接。
- `LegacySceneWorldBuilder.cpp` 已参与编译，当前提供旧 `Scene / Object` 到 `World / Level / Actor` 的非侵入式导入。
- 独立 C++ smoke 已验证旧 `Scene -> Object -> Object` 导入后 Actor 数量、组件 attach 和 transform 正确。
- `WorldLegacySceneExporter.cpp` 已参与编译，当前提供 `World / Level / Actor` 到旧 `Scene` 的非侵入式导出。
- `--verify-engine-world-scene-probe` 已通过，`Engine world prepared scene stats` 输出 `engineWorldProbeMeshes=1`，renderer stats 输出 `pbrGBufferDrawCalls=26`、`pbrDeferredLightingDrawCalls=1`。
- `WorldDrivenSceneSetup.cpp` 已参与编译，当前把 verification-only World probe 抽成 `tools/sceneSetup` 可复用 helper。
- `SceneSetupPipeline.cpp` 已参与编译，`RuntimeScenePreparer` 当前通过 pipeline 默认调用旧 `prepareDefaultScene(...)`。
- `--verify-engine-world-scene-probe` 已通过 `SceneSetupPipelineConfig` 接入 scene prepare 阶段；`Engine world scene probe stats` 只输出 1 次，没有重复添加。
- `SceneSetup.cpp` 已拆出基础设施、旧默认内容和 PBR preview 三个可组合入口，pipeline 可以在不创建旧房间和默认 PBR grid 的情况下准备 renderer、screen pass、environment precompute 和 lights。
- `--verify-engine-world-minimal-scene` 已通过，输出 `Engine world minimal scene stats: actors=5, sceneComponents=5, exportedObjects=5, exportedMeshes=4, sceneRootObjects=1, objectAttachments=4`。
- `--verify-engine-world-minimal-scene` 的 PBR scene stats 显示 `pbrPreviewMeshes=0`，`Engine world prepared scene stats` 显示 `engineWorldMinimalMeshes=4`，renderer stats 显示 `pbrGBufferDrawCalls=4`、`pbrDeferredLightingDrawCalls=1`。
- `AppRuntimeContext` 已新增 `engineWorld`，World-driven scene setup 会填充 runtime 持有的 World 后再导出给旧 renderer。
- `engine-world-scene-probe` 的 `Engine world prepared scene stats` 显示 `runtimeWorldActors=2`，`engine-world-minimal-scene` 显示 `runtimeWorldActors=5`。
- Editor hierarchy 已接入 runtime `engineWorld`，新增只读 `World -> Persistent Level -> Actor -> Component` 树显示和 Actor 选择。
- Editor inspector 已接入只读 Actor inspector，当前展示 Actor 类型、所属 World / Level、组件列表和 root `SceneComponent` transform。
- 默认 legacy scene path 已生成 read-only `engineWorld` mirror；`forward` 的 `Engine world prepared scene stats` 输出 `runtimeWorldActors=33`，legacy mirror stats 输出 `visitedObjects=33`、`actors=33`、`meshActors=32`、`componentAttachments=32`。
- Actor / Component inspector 已接入 `PropertyBuilder / drawProperties(...)` schema，当前统一描述只读 Actor 字段、Component 字段、SceneComponent transform 和 adapter 绑定信息。
- Actor / Component edit commit bridge 已接入：World-driven scene 中具备旧 `Object` adapter 的 `SceneComponent` relative transform 可以通过 schema 写回，并同步旧 renderer 对象 transform；legacy mirror 继续只读。
- Component direct selection 已接入：Engine World hierarchy 中 Component leaf 会进入 `SelectionKind::Component` 并使用独立 Component inspector。
- 最小 edit transaction boundary 已接入：`SceneComponent` transform 写回会记录 target、field、before、after，并标记 dirty；当前只记录，不执行 undo/redo。
- 最小 undo apply 已接入：可以撤销最近一条 `SceneComponent` Vec3 transform transaction，并同步写回 `SceneComponent` 与 adapter 旧对象；当前不支持 redo stack。
- Dirty/save transaction boundary 已接入：inspector 提供 `Mark Saved / Clear Transactions`，当前只管理编辑器 dirty/history 状态，不执行磁盘保存。
- Scene transform snapshot persistence boundary 已接入：可编辑 World-driven scene 可以保存 `SceneComponent` transform snapshot，verification 已断言 snapshot 文件写出。
- Scene transform snapshot load/apply boundary 已接入：snapshot 可以回放到 runtime `World`，并同步旧 renderer adapter；verification 会扰动组件后要求 apply 恢复。
- Stable scene path id 已接入：snapshot 写入 actor/component stable path，apply 优先按 stable path 匹配，并由 verification 断言 `matchedByStablePath>0`。
- Persistent id 字段已接入：`EngineObject` 提供可序列化 persistent id，World-driven scene preset 会写入确定性 persistent id，snapshot apply 优先按 persistent id 匹配，并由 verification 断言 `matchedByPersistentId>0`。
- Legacy mirror persistent id coverage 已接入：旧 `Scene / Object` 树导入 `engineWorld` 时会为 mirror World、Level、Actor 和 root SceneComponent 派生 deterministic persistent id，并由 `ExpectLegacyWorldMirror` 断言覆盖数量。
- Persistent id policy module 已接入：`engine/PersistentIdPolicy` 统一 id 格式为 `objectKind:source:scope:path`，当前已覆盖 `preset`、`legacy-mirror` 与 `imported-asset` 来源。
- Imported asset persistent id coverage 已接入：`--verify-pbr-import` 会把 Assimp PBR import probe 同步导入 runtime `engineWorld`，并由 `ExpectImportedAssetProbe` 断言 `persistentIdSource=imported-asset` 与 Actor / SceneComponent id 覆盖。
- Editor-created persistent id coverage 已接入：`EditorWorldActions::createEditorEmptyActor(...)` 和 `Create Empty Actor` UI 入口会生成 `actor:editor-created:*` / `component:editor-created:*`，并由 `engine-world-editor-create` verification 断言 snapshot 持久化。
- Scene package save/load boundary 已接入：`ScenePackage` 可把最小 `World / Level / Actor / SceneComponent` 层级保存为 `engine.world.scenePackage.v1`，再加载成新的 runtime `World`。
- `--verify-engine-world-scene-package` 已通过 focused verification，输出 `saved=yes`、`loaded=yes`、`loadedActors=5`、`loadedSceneComponents=5`、`loadedActorsWithPersistentIds=5`、`loadedSceneComponentsWithPersistentIds=5`。
- Scene package component type / adapter metadata 已接入：package 会写出 `MeshActor` / `LegacyObjectActor`、`MeshComponent` / `LegacyObjectComponent`、`adapter.kind`、`adapter.assetReference` 和 `adapter.materialType`，load 阶段会恢复 typed component 类。
- Scene package asset resolver / renderer reconstruction 已接入：`loadScenePackage(...)` 可通过 `ScenePackageAssetResolver` 根据 adapter reference 回填 `MeshComponent` / `LightComponent` / `LegacyObjectComponent` 的旧 renderer 对象，并由 verification 断言 `resolvedAdapterReferences=6`、`unresolvedAdapterReferences=0`。
- package-loaded World 已可重新导出到旧 renderer scene：`engine-world-scene-package` mode 断言 loaded World 导出 `exportedLoadedWorldObjects=6`、`exportedLoadedWorldMeshes=4`、`exportedLoadedWorldLights=1`、`exportedLoadedWorldLegacyObjects=1`、`exportedLoadedWorldAttachments=5`。
- AssetRegistry stable handle slice 已接入：`engine/AssetRegistry` 提供 `asset:<kind>:<source>:<path>` handle 工具，scene package 现在写出 `adapter.assetHandle=asset:mesh:runtime-generated:*` / `asset:legacy-object:runtime-generated:*` / `asset:light:runtime-generated:*`，同时保留 `adapter.assetReference=runtime-generated:*` fallback。
- Scene package typed actor restore 已接入：load 阶段会按保存的 actor type 恢复 `MeshActor`、`LightActor`、`LegacyObjectActor` 等动态类型，typed component 统计只记录实际类型匹配。
- `engine-world-scene-package` 已断言 `typedActors=6`、`loadedTypedActors=6`、`assetHandles=6`、`loadedAssetHandles=6`、`resolvedAssetHandles=6`、`unresolvedAssetHandles=0`。
- Scene package negative probes 已接入：loader 会统计 unknown actor/component type 与 invalid transform；verification 已断言 missing schema、invalid line、missing count 被拒绝，unknown type package fallback 成功。
- Imported asset stable handle coverage 已接入：`--verify-pbr-import` 导入 `fbx/test/test.fbx` 时会通过 `AssetRegistry` 注册 `asset:mesh:imported-asset:*`、`asset:material:imported-asset:*`、`asset:texture:imported-asset:*`，并由 verification 断言三类 handle 均存在。
- Imported asset scene package manifest 已接入：`--verify-pbr-import` 会保存并加载 `out/pbr_import_asset_scene_package.verification.ini`，断言 package 文件写出并读回 imported mesh/material/texture manifest。
- Editor asset browser / inspector 可见层已接入：`AppRuntimeContext` 持有 runtime `AssetRegistry`，Editor `asset browser` 面板可列出 imported / all asset handles，点击后通过 `selectedAssetHandle` 在 inspector 中查看 asset descriptor。
- `--verify-pbr-import` 已输出并断言 `Runtime asset registry stats: assets=3, meshAssets=1, materialAssets=1, textureAssets=1, importedAssetHandles=3`。
- Scene package graph validation 已接入：loader 会拒绝 duplicate persistent id、unresolved parent、invalid parent index、self-parent 和 cycle parent，并单独统计合法 cross-actor parent restore。
- `--verify-engine-world-scene-package` 已输出并断言 graph error 计数为 0，negative probes 中 duplicate/unresolved/self/cycle reject 均为 `yes`，`validCrossActorParentLoaded=yes`。
- Scene package asset manifest registry reload 已接入：`loadScenePackage(...)` 可通过 `ScenePackageLoadOptions::assetRegistry` 把 `assetManifest.N.*` 注册回 runtime `AssetRegistry`。
- `--verify-pbr-import` 已断言 imported asset package load 后 `registeredAssetManifest=3`、`registeredMeshAssets=1`、`registeredMaterialAssets=1`、`registeredTextureAssets=1`。
- AssetRegistry subsystem boundary 已接入：新增 `AssetSubsystem : EngineSubsystem`，runtime context 不再直接暴露裸 `AssetRegistry` 字段。
- Engine-owned AssetSubsystem handoff 已接入：`RuntimeApplicationShell` 通过 `Engine::addSubsystem<AssetSubsystem>()` 创建 asset subsystem，runtime context 只保存非拥有 `AssetSubsystem*`，editor/import/package load 通过 Engine-owned subsystem registry 工作。
- `--verify-pbr-import` 已在 Engine-owned AssetSubsystem 路径下通过，继续断言 runtime registry `assets=3`，并断言 package manifest load 后 `registeredAssetManifest=3`。
- Engine-owned World handoff first slice 已接入：`AppRuntimeContext::engineWorld` 改为非拥有 `World*`，scene setup pipeline 通过 `Engine::createWorld(...)` 创建 legacy mirror / World-driven runtime World，再把 active World 指针交给 editor、verification 和 snapshot 系统使用。
- `--verify-pbr` 与 `--verify-engine-world-minimal-scene` 已在 Engine-owned World 路径下通过，分别保持 `runtimeWorldActors=33` 和 `runtimeWorldActors=5`。
- Engine tick frame loop first slice 已接入：`RuntimeFrameRunner` 在旧 renderer frame pipeline 前调用 `Engine::tick(deltaSeconds)`，`Engine::createWorld(...)` 创建的 active World 会 begin play，verification 输出并断言 `Runtime engine tick stats`。
- Runtime frame clock 已接入：普通运行通过 `RuntimeFrameClock` 使用 `std::chrono::steady_clock` 计算 delta，verification 模式仍强制固定 `1.0f / 60.0f`，避免测试抖动。
- RendererSubsystem frame bridge 已接入：`RuntimeApplicationShell` 通过 `Engine::addSubsystem<RendererSubsystem>()` 创建 renderer subsystem，scene prepare 后绑定 runtime renderer，frame runner 在旧 renderer pipeline 前后调用 bridge begin/end。
- verification 已输出并断言 `Runtime renderer subsystem stats`：initialized、hasRenderer、runtimeRendererAttached、frameBridgeActive、begin/completed frame count、observed renderer passes、engine time/delta。
- RendererSubsystem render entry wrapper 已接入：`RuntimeFrameRunner` 通过 `RendererSubsystem::renderFrameBridge(...)` 包裹旧 `RuntimeFramePipeline::render(...)`，并由 verification 断言 `renderFrameBridgeCalls == beginFrames`。
- RendererSubsystem editor stats visibility 已接入：`AppRuntimeContext` 保存非拥有 `RendererSubsystem*`，Debug controller 面板展示 Engine-owned renderer subsystem frame bridge stats，verification 断言 runtime context 暴露的是同一个 subsystem。
- RendererSubsystem frame entry intent 已接入：`RendererSubsystem::renderFrameBridge(...)` 接收 engine-side `RendererFrameIntent`，记录 framebuffer size，并由 verification 断言 `frameConfigValid=yes` 和 framebuffer 尺寸大于 0。
- RendererSubsystem neutral frame pass stats 已接入：`RuntimeFramePipeline::render(...)` 返回 planned/executed/skipped pass 统计，application adapter 转换为 engine-side `plannedPassCount` / `executedPassCount` / `skippedPassCount`，verification 断言 planned == executed + skipped。
- Runtime renderer frame bridge adapter 已接入：application 层新增 `RuntimeRendererFrameBridgeAdapter`，集中负责旧 `RuntimeFramePipeline` stats 到 engine-side `RendererFrameResult` 的转换，`RuntimeFrameRunner` 不再直接依赖 runtime pipeline 实现细节。
- RendererSubsystem frame plan key 已接入：adapter 生成非路径 frame plan token，RendererSubsystem stats / verification / debug UI 均通过 `framePlanKey` 观察该 render intent token，Engine DTO 不再使用 `runtimePipeline*` 字段名。
- RendererSubsystem frame intent/result DTO 已接入：原临时 `RendererSubsystemFrameConfig` / `RendererSubsystemFrameRenderResult` 已整理为 `RendererFrameIntent` / `RendererFrameResult`，接口语义变为“输入 intent、输出 result”。
- Renderer backend interface 已接入：`RendererSubsystem::renderFrameBridge(...)` 不再接收 `std::function` callback，而是依赖 engine-side `RendererBackend`；application adapter 实现该接口并调用旧 `RuntimeFramePipeline`，verification 断言 `frameExecutorAttached=yes` 且 executor call count 与 bridge call count 一致。
- Renderer executor attachment boundary 已接入：`RuntimeApplicationShell` 持有 `RuntimeRendererFrameBridgeAdapter`，并把它作为非拥有 executor 指针附着到 `RendererSubsystem`；`RuntimeFrameRunner` 每帧只提交 frame intent，不再构造 executor。
- Renderer backend metadata contract 已接入：`RendererBackend` 暴露 backend key / readiness，RendererSubsystem stats、Engine diagnostics 和 verification 均可观察并断言 backend 已 ready 且 key 非 `none`。
- Renderer backend lifecycle stats 已接入：RendererSubsystem stats 记录 backend state、attach/detach count、ready/not-ready frame count，verification 断言正常 runtime path 中 backend state 为 ready 且 ready frame count 覆盖所有 render bridge call。
- Runtime renderer backend factory 已接入：`RuntimeApplicationShell` 通过 `RuntimeRendererBackendFactory` 创建 `RendererBackend`，再移交给 Engine-owned `RendererSubsystem`。
- Runtime renderer backend registry 已接入：`RuntimeApplicationShellConfig::rendererBackendKey` 通过 `RuntimeRendererBackendFactory::createBackend(...)` 按 key 选择 backend，RendererSubsystem stats 与 verification 记录 registry key / registry count。
- Renderer backend registry no-op verification 已接入：新增 `test-noop-renderer-backend`，通过 `--verify-renderer-backend-registry-noop` 证明 registry 能切换到不调用旧 `RuntimeFramePipeline` 的第二个 backend。
- Renderer backend Engine-owned ownership 已接入：`RendererSubsystem` 现在通过 `std::unique_ptr<RendererBackend>` 持有 active backend，verification 断言 owner key 为 `engine-renderer-subsystem` 且 ownership 为 `engine-owned`。
- Renderer backend cleanup verification 已接入：verification 现在检查 runtime cleanup 阶段 backend 已 detached，backend key / owner / registry metadata 已清空，attach/detach count 已闭合。
- Renderer backend contract verification naming neutralization 已接入：verification 现在并行输出并断言 `Runtime renderer backend contract stats` / cleanup stats，保留旧 `Runtime renderer subsystem ...` 输出兼容已有脚本视角，同时把 backend key、ready state、registry、frame pass、no-op backend 与 cleanup 状态表述为通用 renderer backend contract，而不是 PBR 专属路径。
- Renderer backend contract header extraction 已接入：新增 `engine/RendererBackend.h`，把 `RendererBackend`、`RendererFrameIntent`、`RendererFrameResult` 和 `RendererBackendAttachmentDesc` 从 `RendererSubsystem.h` 中拆出；application runtime backend 与 factory 现在依赖该窄 contract header，而不是直接包含 subsystem。
- Renderer backend registry metadata contract extraction 已接入：`RendererBackendRegistration` 现在定义在 `engine/RendererBackend.h`，`RuntimeRendererBackendFactory::registeredBackends()` 返回 engine-level registration entry；runtime factory 仍负责创建具体 application backend。
- Renderer backend registry helper extraction 已接入：新增 `engine/RendererBackendRegistry.h/.cpp`，集中 registry query、default backend key 选择和 attachment desc 组装；runtime factory 只保留 runtime backend 列表和具体 backend 创建。
- Renderer backend registry selection policy extraction 已接入：`RendererBackendSelection` 与 `RendererBackendRegistry::resolveBackendSelection(...)` 集中 requested/default/selected/registered 语义；runtime factory 只按 selected key 创建具体 application backend。
- Runtime renderer backend catalog extraction 已接入：新增 `RuntimeRendererBackendCatalog`，集中 runtime backend key、registry metadata、selection entrypoint 和 attachment metadata；`RuntimeRendererBackendFactory` 收敛为只按 `RendererBackendSelection` 创建具体 backend object。
- Runtime Application Config Header extraction 已接入：`RuntimeApplicationShellConfig` 已迁入 `RuntimeApplicationConfig.h`，config/content policy、startup/frame/shutdown lifecycle 和 verification args 不再为了 config 包含完整 Shell header。
- Runtime Application Callback Binder extraction 已接入：新增 `RuntimeApplicationCallbackBinder`，集中 bootstrapper callbacks 到 startup/frame/shutdown lifecycle 的绑定；`RuntimeApplicationShell` 删除私有 lifecycle wrapper，只保留 state/config 与 `makeCallbacks()`。
- Runtime Application Runner Boundary extraction 已接入：新增 `RuntimeApplicationRunner`，集中 `RuntimeApplicationShell` 构造、callback 获取和 `RuntimeBootstrapper::run(...)` 调用；`main.cpp` 不再直接依赖 Shell 或 Bootstrapper。
- Runtime Application Entry Boundary extraction 已接入：新增 `RuntimeApplicationEntry`，集中 logger setup、verification args parsing 和 runner 调用；`PointLightShadow::MAX_POINT_LIGHTS` 静态定义已迁回 light module，`main.cpp` 只保留 entry 委托。
- Runtime Verification Args Neutral Alias extraction 已接入第一片：新增 `RuntimeVerificationArgs.h` 作为 entry-facing neutral public header，`RuntimeApplicationEntry` 不再直接包含 `RuntimePBRVerificationArgs.h`；旧 PBR args header 保留为兼容 wrapper，`.cpp` 实现暂不搬迁。
- Runtime Verification Args Implementation extraction 已接入：`makeShellConfigFromArguments(...)` 实现已迁入 `RuntimeVerificationArgs.cpp`，旧 `RuntimePBRVerificationArgs.cpp` 只保留兼容 stub，内部 descriptor/table 命名已从 PBR-specific 收敛为 runtime verification 语义。
- Runtime Verification Config Field Neutralization 已接入：新增 `RuntimeVerificationConfig` neutral alias，`RuntimeApplicationShellConfig` 字段从 `pbrVerification` 改为 `verification`，application config/content/frame/shutdown/verification lifecycle 与 args parser 均改为消费 `config.verification`。
- Runtime Verification Config Data Model Split 已接入：`RuntimeVerificationConfig` 不再是 `RuntimePBRVerificationConfig` alias，generic lifecycle/capture 字段留在 runtime verification 顶层，PBR probe、Engine World probe 和 renderer timing probe 分别进入 `pbr`、`engineWorld`、`renderer` 子配置。
- Runtime Verification Report extraction 已接入：新增 `RuntimeVerificationReport`，把 Engine lifecycle snapshot、subsystem health、renderer backend contract 和 cleanup report 从 `RuntimePBRVerification` 迁出；PBR 文件只保留 PBR renderer stats report。
- Runtime Engine World Verification extraction 已接入：新增 `RuntimeEngineWorldVerification`，把 editor-create probe、transform snapshot、scene package round-trip、negative package probes 和 package resolver fixture 从 `RuntimePBRVerification` 迁出；PBR 文件保留 PBR scene/import/renderer stats。
- Runtime Engine World Prepared Scene Stats Neutralization 已接入：`PBR verification scene stats` 不再输出 `engineWorldProbeMeshes`、`engineWorldMinimalMeshes` 或 `runtimeWorldActors`；这些 counters 现在由 `Engine world prepared scene stats` 中性行输出并由 verification 脚本断言。
- Runtime Imported Asset Verification extraction 已接入：新增 `RuntimeImportedAssetVerification`，把 imported asset 的 Engine World import、AssetRegistry stats 和 scene package manifest round-trip 从 `RuntimePBRVerification` 迁出，同时保留现有 `--verify-pbr-import` 输出契约。
- Runtime Verification Capture extraction 已接入：新增 `RuntimeVerificationCapture`，把 default framebuffer readback、PPM 写盘和 capture 日志从 `RuntimePBRVerification` 迁出，verification lifecycle 不再通过 PBR 类执行 capture。
- Runtime PBR Renderer Stats Verification extraction 已接入：新增 `RuntimePBRRendererStatsVerification`，把 capture 帧的 `PBR verification renderer stats` 输出从 `RuntimePBRVerification` 迁出，同时保持现有 renderer stats 输出契约。
- Runtime PBR Profile Verification extraction 已接入：新增 `RuntimePBRProfileVerification`，把 PBR startup profile、light/camera rig、frame pipeline profile 和 renderer pass profile 写入从 `RuntimePBRVerification` 迁出。
- Runtime PBR Scene Probe Verification extraction 已接入：新增 `RuntimePBRSceneProbeVerification`，把 transparent fallback、deferred emissive、material IBL、alpha mask、texture set 和 showcase spheres 的 PBR scene probe construction 从 `RuntimePBRVerification` 迁出。
- Runtime PBR Prepared Scene Stats Verification extraction 已接入：新增 `RuntimePBRPreparedSceneStatsVerification`，把 `PBR verification scene stats` 从旧 `RuntimePBRVerification` class 中迁出；旧 `.cpp` 已从工程移除。
- Runtime PBR Verification Config Header Rename 已接入：新增 `RuntimePBRVerificationConfig.h` 承载 `RuntimePBRVerificationConfig`，`RuntimeVerificationConfig.h` 改为包含明确命名的 config header；旧 `RuntimePBRVerification.h` 已从 live code 与 VS 工程注册中移除。
- Runtime PBR Verification Config Pass/Probe/Deferred Split 已接入：`RuntimePBRVerificationConfig` 现在聚合 `passes`、`probes`、`deferred` 三个子配置，args/profile/probe/import verification 不再依赖单个 flat PBR config 字段列表。
- Runtime PBR Pass Profile Verification extraction 已接入：新增 `RuntimePBRPassProfileVerification`，把 renderer pass profile 写入从 `RuntimePBRProfileVerification` 中拆出；prepared-scene lifecycle 可直接刷新 renderer pass profile。
- Runtime PBR Profile Line Verification extraction 已接入：新增 `RuntimePBRProfileLineVerification`，把 `PBR verification profile applied` 输出行从 startup profile 写入中拆出，profile module 不再直接依赖 logger/stdout。
- Runtime PBR Preview Profile Verification extraction 已接入：新增 `RuntimePBRPreviewProfileVerification`，把 PBR preview grid policy 从 startup profile 写入中拆出，保持默认 grid、showcase override 和 minimal-scene disable 行为不变。
- Runtime PBR Light Camera Rig Verification extraction 已接入：新增 `RuntimePBRLightCameraRigVerification`，把 PBR verification 默认、minimal-scene、tiled-light、showcase-camera 和 pressure-light rig policy 从 startup profile composition 中拆出。
- Runtime PBR Startup Profile Verification extraction 已接入：新增 `RuntimePBRStartupProfileVerification`，把 environment、post-process 和 runtime frame pipeline startup defaults 从 `RuntimePBRProfileVerification` 中拆出；`RuntimePBRProfileVerification` 当前只保留 profile application 编排。
- Engine World cleanup verification 已接入：verification 现在检查 runtime cleanup 后 `Engine::shutdown()` 已完成，active World 已 reset，runtime context 中 Engine / World / AssetSubsystem / RendererSubsystem 非拥有指针均已清空。
- Engine Subsystem cleanup verification 已接入：verification 现在检查 `Engine::shutdown()` 后 AssetSubsystem registry/tick 已 reset，RendererSubsystem 不再持有 renderer/executor/backend，且 post-shutdown subsystem 状态仍可通过 Engine-owned 对象只读验证。
- Runtime Engine Lifecycle extraction 已接入：`RuntimeApplicationShell` 不再直接持有 `mAssetSubsystem` / `mRendererSubsystem`，Engine / subsystem / renderer backend lifecycle 编排已迁入 `RuntimeEngineLifecycle`。
- Runtime Verification Lifecycle extraction 已接入：`RuntimeApplicationShell` 不再直接编排 `RuntimePBRVerification::*`，verification mode 的 profile、scene report、capture、cleanup report 和 max-frame stop 条件已迁入 `RuntimeVerificationLifecycle`。
- Runtime Content Lifecycle extraction 已接入：`RuntimeApplicationShell` 不再直接编排 camera/profile/scene prepare/backend attach/prepared scene report，startup content composition 已迁入 `RuntimeContentLifecycle`。
- Runtime Content Lifecycle Config Header extraction 已接入：新增 `RuntimeCameraConfig.h`、`RuntimeScenePrepareConfig.h` 与 `RuntimeContentLifecycleConfig.h`，让 `RuntimeContentLifecycle.h` 不再暴露 camera lifecycle、scene preparer、engine lifecycle、verification lifecycle 或 legacy experiment implementation headers。
- Runtime Content Renderer Backend Lifecycle extraction 已接入：新增 `RuntimeContentRendererBackendLifecycle`，把 scene prepare 后的 renderer fail-fast gate 与 Engine-owned renderer backend attachment 从 generic content composition 中拆出。
- Runtime Content Verification Lifecycle extraction 已接入：新增 `RuntimeContentVerificationLifecycle`，把 runtime profile load、verification startup profile 和 prepared-scene report 从 generic content composition 中拆出。
- Runtime Content Scene Lifecycle extraction 已接入：新增 `RuntimeContentSceneLifecycle`，把 scene preparation stage 从 generic content composition 中拆出。
- Runtime Content Camera Lifecycle extraction 已接入：新增 `RuntimeContentCameraLifecycle`，把 startup camera initialization stage 从 generic content composition 中拆出。
- Runtime Legacy Experiment Lifecycle extraction 已接入：新增 `RuntimeLegacyExperimentLifecycle`，把 legacy experiment context construction、startup enable hooks 和 per-frame update 从 `RuntimeScenePreparer` 中拆出；`RuntimeFrameRunner` 不再为了 legacy update 依赖 scene preparer。
- Scene Setup Pipeline Config Header extraction 已接入：新增 `SceneSetupPipelineConfig.h`，让 `RuntimeScenePrepareConfig.h` 只依赖轻量 pipeline config DTO，不再 public include 完整 `SceneSetupPipeline.h`。
- Runtime Scene Setup Report extraction 已接入：新增 `RuntimeSceneSetupReport`，把 scene setup result stdout/logger 输出和 renderer prepared 日志从 `RuntimeScenePreparer` 中拆出；`RuntimeScenePreparer` 不再直接依赖 logger/stdout 或 scene setup stats formatter。
- Runtime Scene Setup Context Factory extraction 已接入：新增 `RuntimeSceneSetupContextFactory`，把 `AppRuntimeContext` 到 `GL_SCENE::SetupContext` 的字段展开从 `RuntimeScenePreparer` 中拆出；`RuntimeScenePreparer` 不再公开 `makeSceneSetupContext(...)`。
- Runtime Scene Setup Pipeline Lifecycle extraction 已接入：新增 `RuntimeSceneSetupPipelineLifecycle`，把 setup context factory、`GL_SCENE::prepareScene(...)` 和 prepared-scene setup report 从 `RuntimeScenePreparer` 中拆出。
- Runtime Scene Preparer removal 已接入：删除无状态 `RuntimeScenePreparer.h/.cpp` wrapper，`RuntimeContentSceneLifecycle` 直接拥有 scene preparation sequence。
- Runtime Application Content Startup Lifecycle extraction 已接入：新增 `RuntimeApplicationContentStartupLifecycle`，把 startup lifecycle 中 content config policy 与 `RuntimeContentLifecycle::prepare(...)` 桥接拆出。
- Runtime Application Editor Startup Lifecycle extraction 已接入：新增 `RuntimeApplicationEditorStartupLifecycle`，把 startup lifecycle 中 editor config policy 与 `RuntimeEditorLifecycle::initialize(...)` 桥接拆出。
- Runtime Application Frame Startup Lifecycle extraction 已接入：新增 `RuntimeApplicationFrameStartupLifecycle`，把 startup lifecycle 中 `RuntimeFrameLifecycle::reset(...)` 桥接拆出。
- Runtime Application Engine Startup Lifecycle extraction 已接入：新增 `RuntimeApplicationEngineStartupLifecycle`，把 startup lifecycle 中 Engine desc mapping 与 `RuntimeEngineLifecycle::initializeEngine(...)` 桥接拆出。
- Runtime Application Window Startup Lifecycle extraction 已接入：新增 `RuntimeApplicationWindowStartupLifecycle`，把 startup lifecycle 中 window setup prompt、window initialization 和 window snapshot 桥接拆出。
- Runtime Application Graphics Startup Lifecycle extraction 已接入：新增 `RuntimeApplicationGraphicsStartupLifecycle`，把 startup lifecycle 中 graphics config mapping 与 `RuntimeGraphicsLifecycle::initializeAfterWindow(...)` 桥接拆出。
- Runtime Application Frame Editor Callback Bridge extraction 已接入：新增 `RuntimeApplicationFrameEditorCallbackBridge`，把 frame lifecycle 中 editor callback config mapping 与 `RuntimeEditorLifecycle::makeFrameCallbacks(...)` 桥接拆出。
- Runtime Application Frame Continue/Run Bridge extraction 已接入：新增 `RuntimeApplicationFrameContinueBridge` 与 `RuntimeApplicationFrameRunBridge`，把 frame lifecycle 中 continue config mapping、window snapshot 和 frame execution 参数展开桥接拆出。
- Runtime Application Shutdown Cleanup/Destroy Bridge extraction 已接入：新增 `RuntimeApplicationShutdownCleanupBridge` 与 `RuntimeApplicationShutdownDestroyBridge`，把 shutdown lifecycle 中 cleanup sequence 和 window destroy 细节桥接拆出。
- Runtime Application Shutdown Verification Bridge extraction 已接入：新增 `RuntimeApplicationShutdownVerificationBridge`，把 shutdown cleanup 中 renderer subsystem cleanup report 与 Engine cleanup report 从 cleanup sequence bridge 中拆出。
- Runtime Application Shutdown Engine Bridge extraction 已接入：新增 `RuntimeApplicationShutdownEngineBridge`，把 shutdown cleanup 中 begin cleanup、camera cleanup、runtime context detach 和 Engine shutdown 从 cleanup sequence bridge 中拆出；早期 application-level cleanup refs DTO 已在后续 canonicalization 中由 `RuntimeEngineLifecycleCleanupRefs` 取代。
- Runtime Engine Lifecycle Types Header extraction 已接入：最初新增 `RuntimeEngineLifecycleTypes.h`，把 `RuntimeEngineLifecycleState` 与 `RuntimeEngineLifecycleCleanupRefs` 从 `RuntimeEngineLifecycle.h` 中拆出；当前已进一步被 `RuntimeEngineLifecycleState.h` 与 `RuntimeEngineLifecycleCleanupRefs.h` 两个窄头取代。
- Runtime Application State Forward Boundary 已接入：`RuntimeApplicationShell.h` 通过 `std::unique_ptr<RuntimeApplicationState>` 隐藏完整 state 类型，application startup/frame/shutdown bridge public headers 改为前置声明 state/config，完整依赖局部化到 `.cpp`。
- Runtime Profile State extraction 已接入：新增 `RuntimeProfileState.h`，把 frame pipeline、post-process、environment、PBR preview/light/camera profile 与 profile paths 从 `AppRuntimeContext` 中聚合为 `context.profiles` 子边界。
- Runtime Render Resource State extraction 已接入：新增 `RuntimeRenderResourceState.h`，把 renderer、screen/world scenes、frame render targets、bloom、runtime meshes/materials、post-process pass 与 clear color 从 `AppRuntimeContext` 中聚合为 `context.renderResources` 子边界。
- Runtime Camera Light State extraction 已接入：新增 `RuntimeCameraLightState.h`，把 camera、camera control、ambient/directional/spot/point light state 从 `AppRuntimeContext` 中聚合为 `context.cameraLights` 子边界。
- Runtime Engine Attachment State extraction 已接入：新增 `RuntimeEngineAttachmentState.h`，把 Engine、World、AssetSubsystem、RendererSubsystem 与 engine-world editable flag 从 `AppRuntimeContext` 中聚合为 `context.engineAttachments` 子边界。
- Runtime Renderer Backend Attachment Lifecycle extraction 已接入：新增 `RuntimeRendererBackendAttachmentLifecycle`，把 renderer backend selection、factory create、attachment desc 组装和 `RendererSubsystem::setRendererBackend(...)` 从 `RuntimeEngineLifecycle.cpp` 中拆出。
- RendererSubsystem Frame Bridge Stats Header extraction 已接入：新增 `RendererSubsystemFrameBridgeStats.h`，把 frame bridge stats DTO 字段列表从 `RendererSubsystem.h` 中拆出，subsystem header 不再直接承载 `<string>` stats 依赖。
- RendererSubsystem Frame Bridge State extraction 已接入：新增 `RendererSubsystemFrameBridgeState`，集中 frame bridge stats 写入策略，`RendererSubsystem` 不再直接写 stats 字段。
- RendererSubsystem Backend Slot extraction 已接入：新增 `RendererSubsystemBackendSlot`，把 backend pointer storage、attachment metadata normalization、backend ready 判断和 attach/detach change detection 从 `RendererSubsystem` 中拆出。
- RendererSubsystem Backend Slot Snapshot extraction 已接入：新增 `RendererSubsystemBackendSlotSnapshot`，让 frame bridge state 通过 backend slot snapshot 刷新 backend stats，不再直接探测 raw backend。
- RendererSubsystem Frame Execution Bridge extraction 已接入：新增 `RendererSubsystemFrameExecutionBridge`，把 backend frame execution / default frame result generation 从 `RendererSubsystem` 中拆出，subsystem 仍保留 lifecycle 与 stats counter coordination。
- RendererSubsystem Implementation State Header Boundary Cleanup 已接入：`RendererSubsystem.h` 用 private owning pointers 隐藏 backend slot、frame execution bridge 和 frame bridge state 实现成员，完整实现头依赖下沉到 `RendererSubsystem.cpp`；需要读取 stats 或析构 backend 的调用点改为显式 include 对应完整类型头。
- RendererSubsystem Renderer Backend API Naming Cleanup 已接入：live application code 改用 `setRendererBackend(...)` / `hasRendererBackend()` / `clearRendererBackend()`；verification 新增 `rendererBackendAttached` / `rendererBackendFrameCalls` 字段并保留旧 `frameExecutor*` 兼容字段。
- RendererSubsystem Renderer Backend Public Compatibility API Removal 已接入：删除旧 `RendererFrameExecutor` / `RendererSubsystemFrameIntent` / `RendererSubsystemFrameResult` aliases 和 `setFrameExecutor(...)` / `hasFrameExecutor()` public wrappers；旧 `frameExecutor*` 仅作为 verification/log compatibility 字段保留。
- RendererSubsystem Frame Executor Stats Internal Compatibility Removal 已接入：Engine 内部 stats 不再保存 `frameExecutorAttached` / `frameExecutorCallCount`，旧 `frameExecutor*` verification/log 输出由 `rendererBackend*` stats 派生。
- RendererSubsystem Frame Executor Verification Output Removal 已接入：`RuntimeVerificationReport` 与 `verify_pbr.ps1` 不再输出、解析或断言旧 `frameExecutor*` 字段，renderer backend contract 只保留 `rendererBackend*` verification 字段。
- Runtime Renderer Backend Verification Report Formatter extraction 已接入：新增 `RuntimeRendererBackendVerificationReport`，把四类 runtime renderer backend verification 行格式化从 `RuntimeVerificationReport` 中拆出，保持 stdout/logger 输出所有权和 verification parser contract 不变。
- Runtime Engine Verification Report Formatter extraction 已接入：新增 `RuntimeEngineVerificationReport`，把 Engine lifecycle snapshot、subsystem summary、tick health、Engine World cleanup 和 Engine subsystem cleanup 行格式化从 `RuntimeVerificationReport` 中拆出，保持 stdout/logger 输出所有权和 verification parser contract 不变。
- Runtime Verification Frame Capture Lifecycle extraction 已接入：新增 `RuntimeVerificationFrameCaptureLifecycle`，把 framebuffer capture、runtime Engine/renderer report 和 PBR renderer stats report 编排从 `RuntimeVerificationLifecycle` 中拆出，保持 public API、capture gate 和 verification 输出合同不变。
- Runtime Verification Prepared Scene Lifecycle extraction 已接入：新增 `RuntimeVerificationPreparedSceneLifecycle`，把 Engine World probe、PBR scene probe、imported asset probe、renderer pass profile 和 prepared-scene stats report 编排从 `RuntimeVerificationLifecycle` 中拆出，保持 public API、输出顺序和 verification parser contract 不变。
- Runtime Verification Cleanup Lifecycle extraction 已接入：新增 `RuntimeVerificationCleanupLifecycle`，把 renderer subsystem cleanup、Engine World cleanup 和 Engine subsystem cleanup report 编排从 `RuntimeVerificationLifecycle` 中拆出，保持 public API、cleanup ordering 和 verification parser contract 不变。
- Runtime Verification Startup Profile Lifecycle extraction 已接入：新增 `RuntimeVerificationStartupProfileLifecycle`，把 startup profile gate 与 PBR profile application 从 `RuntimeVerificationLifecycle` 中拆出，保持 public API、profile applied 输出和 verification parser contract 不变。
- Runtime Verification Stop Policy extraction 已接入：新增 `RuntimeVerificationStopPolicy`，把 verification max-frame stop condition 从 `RuntimeVerificationLifecycle` 中拆出，保持 public API、frame loop 和 verification max-frame 语义不变。
- Runtime Verification Lifecycle Header Forward Boundary 已接入：`RuntimeVerificationLifecycle.h` 不再传递 include `AppRuntimeContext.h` 或 `RuntimeVerificationConfig.h`，只保留 facade API 所需的 forward declarations；需要完整 config 的 `RuntimeFrameLifecycle.h` 改为显式 include。
- Runtime Frame Callbacks Header extraction 已接入：新增 `RuntimeFrameCallbacks.h`，让 frame lifecycle、frame runner 与 editor callback bridge 共用轻量 callback DTO，降低 `RuntimeFrameLifecycle.h` 对 runner/context/legacy experiment runner 的传递依赖。
- Runtime Frame Callback Default Argument Header Boundary Cleanup 已接入：`RuntimeFrameRunner.h` 与 `RuntimeFrameLifecycle.h` 用无 callback overload 替代 `RuntimeFrameCallbacks` 默认参数，public headers 只 forward declare callback DTO；完整 callback include 局部化到 runner/lifecycle implementation 和实际构造 editor frame callbacks 的 frame run bridge。
- Runtime Frame Lifecycle Types Header extraction 已被后续窄头拆分取代：frame lifecycle config/state DTO 已从 `RuntimeFrameLifecycle.h` 中拆出，facade header 只保留行为入口和 forward declaration；当前 canonical 入口是 `RuntimeFrameLifecycleConfig.h` 与 `RuntimeFrameLifecycleState.h`。
- Runtime Application Frame Editor Callback Bridge Header Boundary 已接入：`RuntimeApplicationFrameEditorCallbackBridge.h` 不再 include `RuntimeFrameCallbacks.h`，只 forward declare callback DTO，完整依赖局部化到 `.cpp`。
- Runtime Frame Runner Types Header extraction 已接入：新增 `RuntimeFrameRunnerTypes.h`，把 `RuntimeFrameConfig` 从 `RuntimeFrameRunner.h` 中拆出，runner facade header 只保留 run entry、callback DTO 和 config forward declaration。
- Runtime Application Frame Bridge Implementation Include Cleanup 已接入：frame continue/run bridge headers 已是 forward boundary，`RuntimeApplicationFrameLifecycle.cpp`、`RuntimeApplicationFrameContinueBridge.cpp` 与 `RuntimeApplicationFrameRunBridge.cpp` 移除冗余完整 config/state include。
- Runtime Application Shutdown Bridge Implementation Include Cleanup 已接入：shutdown lifecycle facade `.cpp` 与 cleanup bridge `.cpp` 移除冗余完整 config/state include，verification bridge 保留真正读取 `config.verification` 所需 include。
- Runtime Application Startup Bridge Include Surface Cleanup 已接入：startup facade/content/editor/engine implementation 移除冗余完整 config/state include，graphics startup public header 改为 forward declare shell config。
- Runtime Application Shell Config Header Boundary Cleanup 已接入：`RuntimeApplicationShell.h` 不再 include 完整 `RuntimeApplicationConfig.h`，shell config 改由 private owning pointer 持有；完整 config 依赖局部化到 shell implementation 和 runner composition root。
- Runtime Application Config Policy Header Include Surface Cleanup 已接入：`RuntimeApplicationConfigPolicy.h` 改为 forward declaration boundary，完整 config/lifecycle/EngineContext 依赖局部化到 `.cpp` 与需要完整 `EngineDesc` 的调用点。
- Runtime Content Config Policy Header Include Surface Cleanup 已接入：`RuntimeContentConfigPolicy.h` 改为 forward declaration boundary，完整 shell/content config 依赖局部化到 `.cpp` 与显式消费返回临时对象的 content startup 调用点。
- Runtime Content Lifecycle Header Config Forward Boundary Cleanup 已接入：`RuntimeContentLifecycle.h` 改为 forward declare `RuntimeContentLifecycleConfig`，完整 config DTO 依赖局部化到 lifecycle implementation 与 content startup 调用点。
- Runtime Content Camera/Scene Lifecycle Header Config Forward Boundary Cleanup 已接入：`RuntimeContentCameraLifecycle.h` 与 `RuntimeContentSceneLifecycle.h` 改为 forward declare 对应 config DTO，完整 DTO 依赖局部化到各自 implementation。
- Runtime Application Shutdown Cleanup Refs Canonicalization 已接入：application shutdown bridge 不再定义重复的 cleanup refs DTO，直接复用 `RuntimeEngineLifecycleCleanupRefs` 作为 shutdown cleanup / verification report 的 canonical refs。
- Runtime Application Shutdown Verification Config Boundary Cleanup 已接入：shutdown verification bridge 改为接收 `RuntimeVerificationConfig`，不再为了 cleanup report include 完整 `RuntimeApplicationConfig.h`。
- Engine Public Header Context Ownership Boundary Cleanup 已接入：`Engine.h` 不再 public include `EngineContext.h` / `World.h`，完整 context/world 依赖集中到 `Engine.cpp`，`EngineContext` 改由 Engine 通过 `std::unique_ptr` out-of-line 构造。
- Engine AddSubsystem Context Helper Boundary Cleanup 已接入：`Engine::addSubsystem(...)` public template 不再直接解引用 `mContext`，initialized-subsystem context handoff 下沉到 `Engine.cpp` 私有 helper。
- Engine World Persistent Level Header Boundary Cleanup 已接入：`World.h` 不再 include 完整 `Level.h`，persistent level 通过 forward declaration + out-of-line destructor 隐藏；实际构造/遍历 Level 的实现文件显式 include `Level.h`。
- Engine Actor Root SceneComponent Header Boundary Cleanup 已接入：`Actor.h` 不再 include 完整 `SceneComponent.h`，root component pointer API 改由 forward declaration 暴露；`Actor.cpp` 显式 include `SceneComponent.h` 以支持 register/dynamic_cast 逻辑。
- Engine Level Actor Header Boundary Cleanup 已接入：`Level.h` 不再 include 完整 `Actor.h`，actor owner 列表通过 forward declaration + out-of-line destructor 隐藏；实际遍历/生命周期调用 actor 的 implementation 显式 include `Actor.h`，`spawnActor<T>` 调用点继续由具体 actor 类型 include 保障。
- Engine Legacy Scene Transform Header Boundary Cleanup 已接入：`LegacySceneWorldBuilder.h` 与 `WorldLegacySceneExporter.h` 不再 include 完整 `Transform.h`，Transform 只通过 forward declaration 暴露；实际读取/写入 transform 字段的 import/export implementation 显式 include `Transform.h`。
- Engine ScenePackage Load Result World Owner Boundary Cleanup 已接入：`ScenePackageLoadResult` 的 `std::unique_ptr<World>` 特殊成员改为 out-of-line default，完整 `World.h` 依赖保持在 `ScenePackage.cpp`，load result 继续保持 move-only 返回语义。
- Renderer Backend Contract Frame DTO Header Boundary Cleanup 已接入：`RendererBackend.h` 不再 include 完整 `RendererBackendFrameTypes.h`，backend contract 只 forward declare `RendererFrameIntent` / `RendererFrameResult`；实际读取 frame intent 或构造 frame result 的 runtime backend implementation 显式 include DTO 头。
- Runtime Frame Pipeline Context Header Boundary Cleanup 已接入：`RuntimeFramePipeline.h` 与 `RuntimeFramePasses.h` 不再 include 完整 `AppRuntimeContext.h`，frame pipeline/pass public headers 只保留 runtime context/config forward declarations；实际读取 context/config 字段的 implementation 显式 include 完整头。
- Runtime Frame Pass Registry Key String View Boundary Cleanup 已接入：`RuntimeFramePassRegistry.h` 的 pass key lookup 改为 `std::string_view`，registry public header 不再为了只读 key 查询 include `<string>`；trim/token 字符串处理保留在 implementation。
- Engine Lifecycle Snapshot Header Boundary Cleanup 已接入：`Engine.h` 不再 include 完整 `EngineLifecycleSnapshot.h`，只 forward declare `EngineLifecycleSnapshot`；实际构造或读取快照字段的 `Engine.cpp`、Engine diagnostics panel 和 runtime verification report 显式 include 完整快照头。
- Frame Render Targets Framebuffer Header Boundary Cleanup 已接入：`FrameRenderTargets.h` 不再 include 完整 `framebuffer/framebuffer.h`，只 forward declare `Framebuffer` / `Texture`；实际 FBO 创建、FBO id 查询和 attachment 访问集中到 `FrameRenderTargets.cpp`。
- PostProcess Pass Header Boundary Cleanup 已接入：`PostProcessPass.h` 不再 include 完整 framebuffer/mesh/shader/settings headers，只 forward declare 引用参数类型；post-process resolve/composite/bloom 执行依赖集中到 `PostProcessPass.cpp`。
- Bloom Header Framebuffer Boundary Cleanup 已接入：`Bloom.h` 不再 include 完整 core/framebuffer/geometry/shader headers，只 forward declare `Framebuffer` / `Texture` / `Shader` / `Geometry`；Bloom FBO 创建、texture binding 和 shader/quad 操作集中到 `Bloom.cpp`。
- Environment Texture Header Boundary Cleanup 已接入：`EnvironmentRenderTargets.h` 与 `EnvironmentProfile.h` 不再 include 完整 `framework/texture.h`，只 forward declare `Texture`；环境贴图创建、HDR/procedural texture 创建、IBL debug/precompute 和 PBR IBL/deferred lighting 中实际解引用 texture 的 implementation 显式 include 完整 texture 头。
- IBL Precompute Pass Header Boundary Cleanup 已接入：`IBLPrecomputePass.h` 不再 include 完整 environment targets、texture、mesh、shader library headers，也不再通过 private helper 暴露 `glm::mat4`；capture projection/view helper 和完整 IBL precompute 执行依赖集中到 `IBLPrecomputePass.cpp`。
- Light Resource Binder Header Boundary Cleanup 已接入：`LightResourceBinder.h` 不再 include 完整 `framework/shader.h` 或 light implementation headers，只 forward declare shader/light 参数类型；实际 uniform 写入和 light 字段读取依赖集中到 `LightResourceBinder.cpp`。
- Depth Prepass Binder Header Boundary Cleanup 已接入：`DepthPrepassBinder.h` 不再 include 完整 `framework/shader.h`、`mesh/mesh.h` 或 `MaterialBindingContext.h`，只 forward declare 参数类型；实际 depth frame/object uniform 绑定依赖集中到 `DepthPrepassBinder.cpp`。
- Material Binder Header Boundary Cleanup 已接入：`MaterialBinder.h` 不再 include 完整 `framework/shader.h`、`materials/material.h`、`mesh/mesh.h` 或 `MaterialBindingContext.h`，只 forward declare 参数类型；实际材质分发、texture binding、shader uniform 和 mesh 访问依赖集中到 `MaterialBinder.cpp`。
- PBR Material Binder Header Boundary Cleanup 已接入：`PBRMaterialBinder.h` 不再 include 完整 `framework/shader.h`、`PBRMaterial.h`、`mesh/mesh.h` 或 `MaterialBindingContext.h`，只 forward declare 参数类型；实际 PBR object/light/shadow/surface/IBL binder 编排依赖集中到 `PBRMaterialBinder.cpp`。
- PBR Object Uniform Binder Header Boundary Cleanup 已接入：`PBRObjectUniformBinder.h` 不再 include 完整 `framework/shader.h`、`PBRMaterial.h`、`mesh/mesh.h` 或 `MaterialBindingContext.h`，只 forward declare 参数类型；实际 model/view/projection/normal matrix、camera position、opacity/time uniform 写入依赖集中到 `PBRObjectUniformBinder.cpp`。
- PBR Shadow Resource Binder Header Boundary Cleanup 已接入：`PBRShadowResourceBinder.h` 不再 include 完整 `framework/shader.h` 或 `MaterialBindingContext.h`，只 forward declare 参数类型；实际 CSM shadow、PBR shadow atlas、point shadow atlas、camera 和 shader uniform 依赖集中到 `PBRShadowResourceBinder.cpp`。
- PBR Surface Resource Binder Header Boundary Cleanup 已接入：`PBRSurfaceResourceBinder.h` 不再 include 完整 `framework/shader.h` 或 `PBRMaterial.h`，只 forward declare 参数类型；实际 surface uniform、texture slot、texture binding 和 shader/material 依赖集中到 `PBRSurfaceResourceBinder.cpp`。
- PBR IBL Resource Binder Header Boundary Cleanup 已接入：`PBRIBLResourceBinder.h` 不再 include 完整 `framework/shader.h` 或 `PBRMaterial.h`，只 forward declare shader、PBR material 和 environment targets 参数类型；实际 IBL readiness 判断、IBL float slot、environment target 和 texture binding 依赖集中到 `PBRIBLResourceBinder.cpp`。
- Shadow Resource Binder Header Boundary Cleanup 已接入：`ShadowResourceBinder.h` 不再 include 完整 `camera/camera.h`、`framework/shader.h`、`directionalLight.h` 或 `pointLight.h`，只 forward declare 全局 `Camera` 以及 shader/light 参数类型；实际 CSM、point shadow、fallback directional shadow、shader uniform 和 light/camera 字段读取依赖集中到 `ShadowResourceBinder.cpp`。
- PBR Alpha Shadow Binder Header Boundary Cleanup 已接入：`PBRAlphaShadowBinder.h` 不再 include 完整 `framework/shader.h` 或 `mesh/mesh.h`，只 forward declare shader/mesh 参数类型并显式 include glm 类型头；实际 alpha-masked PBR mesh 判断、shader uniform、mesh/material/texture 依赖集中到 `PBRAlphaShadowBinder.cpp`。
- Shadow Render Pass Header Boundary Cleanup 已接入：`DirectionalShadowRenderPass.h` 与 `PointShadowRenderPass.h` 不再 include 完整 camera/light/mesh/shader-library headers，只 forward declare 参数类型并保留轻量 stats 头；实际 shadow framebuffer、camera/light 字段、mesh draw、shader uniform 和 alpha-shadow 分支依赖集中到各自 `.cpp`。
- Draw Helper Debug Quad Header Boundary Cleanup 已接入：`MeshDraw.h`、`ShadowMeshDraw.h` 和 IBL/GBuffer/tiled/clustered debug quad pass headers 不再 include 完整 `mesh/mesh.h` 或 `materials/material.h`，只 forward declare mesh 参数/成员类型；实际 mesh draw、screen quad 构造和 material type 判断依赖集中到对应 `.cpp`。
- PBR Draw Pass Header Boundary Cleanup 已接入：`PBRDepthPrepass.h`、`PBRGBufferPass.h` 与 `PBRSceneRenderPass.h` 不再 include 完整 mesh、material binding context 或 shader library headers，只 forward declare 参数类型；实际 PBR material 判断、shader uniform、G-buffer target 和 mesh draw 依赖集中到各自 `.cpp`。
- Scene Render Pass Header Boundary Cleanup 已接入：`SceneRenderPass.h` 不再 include 完整 material、mesh、material binding context、shader 或 shader library headers，只 forward declare 参数类型；实际 legacy material 选择、render state、shader binding 和 mesh draw 依赖集中到 `SceneRenderPass.cpp`。
- PBR Shadow Atlas Render Pass Header Boundary Cleanup 已接入：`PBRShadowAtlasRenderPass.h` 不再 include 完整 camera、directional/point light、mesh 或 shader library headers，只保留 atlas stats/target 类型头和参数前置声明；实际 CSM cascade、point shadow cubemap face、alpha-mask shadow shader、mesh draw 和 shader uniform 依赖集中到 `PBRShadowAtlasRenderPass.cpp`。
- PBR Deferred Lighting Grid Header Boundary Cleanup 已接入：`PBRDeferredLightingPass.h`、`PBRDeferredLightBuffer.h`、`PBRDeferredTiledLightGrid.h` 与 `PBRDeferredClusteredLightGrid.h` 不再 include 完整 material binding context、mesh 或 GL core headers，只保留必要的 light culling config/stats/value-member 类型与 `glm::ivec4` 轻量头；实际 context 字段读取、light packing、CPU tiled/clustered grid 构建、GPU clustered dispatch、lighting quad mesh 和 GL buffer 操作依赖集中到对应 `.cpp`。
- Renderer Infrastructure and Runtime Input Header Boundary Cleanup 已接入：`RenderQueue.h`、`ShadowRenderer.h`、`ShaderLibrary.h`、`PBRShadowAtlasRenderTargets.h` 与 `RuntimeInputController.h` 不再传播 camera/scene/mesh/shader/core/camera-control 等 implementation-only headers；实际 render queue projection/sort、shadow renderer pass dispatch、shader construction、shadow atlas GL texture allocation 和 input controller camera/control 操作依赖集中到对应 `.cpp`。
- Renderer Facade PImpl Header Boundary Cleanup 已接入：`renderer.h` 不再传播 render pass、queue、shadow renderer、shader library、render target、scene/camera/light/mesh/shader/core 等 implementation-only headers；`Renderer` 内部渲染状态迁入 `Renderer::Impl`，完整依赖集中到 `renderer.cpp`，实际读取 frame stats/profile/environment/scene 的 runtime/editor 调用点改为显式 include。
- SceneSetup Context Header Boundary Cleanup 已接入：`SceneSetup.h` 不再传播 scene/light/material/mesh/Bloom/environment/frame-target/renderer/profile 完整 headers，只保留 `SetupContext` 引用/shared_ptr 契约和前置声明；实际 scene setup 构造、pipeline context 字段读取和 runtime lifecycle 按值持有完整 context 的依赖集中到对应 `.cpp`。
- Engine AssetSubsystem Registry Header Boundary Cleanup 已接入：`AssetSubsystem.h` 不再 include 完整 `AssetRegistry.h`，registry 通过 private owning pointer 隐藏；实际访问 registry API 的实现文件显式 include `AssetRegistry.h`。
- Runtime Application Shutdown Cleanup Verification Config Boundary Cleanup 已接入：shutdown cleanup bridge 改为接收 `RuntimeVerificationConfig`，完整 shell config 到 verification config 的映射集中到 shutdown lifecycle facade。
- Runtime Application Shutdown Lifecycle Verification Config Boundary Cleanup 已接入：shutdown lifecycle facade 改为接收 `RuntimeVerificationConfig`，完整 shell config 到 verification config 的映射上移到 callback binder。
- Runtime Application Callback Binder Shutdown Bridge Boundary Cleanup 已接入：新增 shutdown callback bridge，callback binder 不再直接 include shutdown lifecycle 或 config policy，shutdown callback 的 verification config 映射集中到 bridge implementation。
- Runtime Application Callback Binder Frame Bridge Boundary Cleanup 已接入：新增 frame callback bridge，callback binder 不再直接 include frame lifecycle，frame shouldContinue/runFrame 转发集中到 bridge implementation。
- Runtime Application Callback Binder Startup Bridge Boundary Cleanup 已接入：新增 startup callback bridge，callback binder 不再直接 include startup lifecycle，startup initialize 转发集中到 bridge implementation。
- Runtime Bootstrapper Callbacks Header Extraction 已接入：新增 `RuntimeBootstrapperCallbacks.h` 承载 callback DTO，`RuntimeBootstrapper.h` 只保留 runner facade 与 DTO 前置声明；callback binder / shell 只依赖 DTO 头，runner 同时显式依赖 runner facade 与 DTO。
- Runtime Engine Lifecycle Header Type Include Surface Cleanup 已接入：`RuntimeEngineLifecycle.h` 改为 forward declare lifecycle state/cleanup refs；完整 state/cleanup refs 依赖当前已局部化到 implementation、state owner、frame 字段读取和 cleanup refs 字段访问方。
- Runtime Renderer Backend Key String View Boundary Cleanup 已接入：renderer backend key 在 content/engine/attachment/catalog/registry 只读传递路径中改为 `std::string_view`，config DTO、selection 和 attachment desc 仍保留 `std::string` 持久化字段。
- Renderer Backend Registry Types Header Extraction 已接入：新增 `RendererBackendRegistryTypes.h` 承载 attachment desc、registration 与 selection metadata，registry/catalog/factory metadata 路径不再通过完整 `RendererBackend.h` 传递依赖。
- Renderer Backend Frame Types Header Extraction 已接入：新增 `RendererBackendFrameTypes.h` 承载 `RendererFrameIntent` / `RendererFrameResult`，frame bridge public headers 不再为了 frame DTO 或 backend slot snapshot 传递完整 `RendererBackend.h` / `RendererSubsystemBackendSlot.h`。
- RendererSubsystem Backend Slot Header Boundary Cleanup 已接入：`RendererSubsystemBackendSlot.h` 不再 include 完整 `RendererBackend.h`，backend owner 通过 forward declaration + out-of-line destructor 隐藏；实际调用 backend virtual API 的 implementation 显式 include `RendererBackend.h`。
- Runtime Renderer Backend Catalog Registry Header Boundary Cleanup 已接入：`RuntimeRendererBackendCatalog.h` 不再 include 完整 `RendererBackendRegistry.h`，catalog public surface 只传播 registry DTO/types 与 registry forward declaration；完整 registry 构造和查询依赖局部化到 catalog implementation。
- Runtime Renderer Backend Catalog Registry Object API Cleanup 已接入：`RuntimeRendererBackendCatalog.h` 不再公开返回具体 `RendererBackendRegistry` object 的 `makeRegistry()` API；registry 构造收敛为 catalog implementation-local helper，public facade 只保留轻量 key/query/selection/attachment API。
- Runtime Renderer Backend Keys Header Extraction 已接入：新增 `RuntimeRendererBackendKeys.h` 承载 runtime/default/no-op backend key helper，config、verification args 与 factory implementation 不再为了 key 常量依赖 catalog/registry。
- Runtime Window Lifecycle Types Header Extraction 已接入：新增 `RuntimeWindowLifecycleTypes.h` 承载 window config/snapshot/callback DTO，application config 不再为了 `RuntimeWindowConfig` 间接包含完整 window lifecycle 和 runtime context。
- Runtime Frame Clock Config Header Extraction 已接入：新增 `RuntimeFrameClockTypes.h` 承载 `RuntimeFrameClockConfig`，application config 不再为了 frame clock config 间接包含完整 frame clock 行为头和 `<chrono>`。
- Runtime Frame Lifecycle Config/State Header Split 已接入：新增 `RuntimeFrameLifecycleConfig.h` 与 `RuntimeFrameLifecycleState.h`，只构造 frame lifecycle config 的路径不再被 state 的完整 frame clock 行为头和 `<chrono>` 依赖污染。
- Runtime Frame Lifecycle Types Compatibility Aggregator Removal 已接入：无源码 include 的旧 frame lifecycle config/state 聚合头已删除，VS project/filter 注册同步移除，`RuntimeFrameLifecycleConfig.h` 与 `RuntimeFrameLifecycleState.h` 保持唯一 canonical 窄头入口。
- Runtime Application Public Header Include Boundary Cleanup 已接入：`RuntimeApplicationCallbackBinder.h` 不再传递 `RuntimeBootstrapper.h`，`RuntimeApplicationWindowStartupLifecycle.h` 不再传递 `RuntimeWindowLifecycleTypes.h`，完整 callback/window DTO 依赖局部化到实际构造或按值返回的 implementation。
- Runtime PBR Verification Config Include Boundary Cleanup 已接入：PBR profile facade 和 prepared-scene stats implementation 不再 include 完整 `RuntimeVerificationConfig.h`；完整 config 依赖保留在实际读取字段的 PBR verification 子模块。
- Runtime Editor Lifecycle Callback DTO Header Boundary Cleanup 已接入：`RuntimeEditorLifecycle.h` 不再 include `RuntimeFrameCallbacks.h`，只 forward declare callback DTO；完整 callback 依赖局部化到 `RuntimeEditorLifecycle.cpp` 和 frame editor callback bridge implementation。
- Engine Lifecycle Snapshot Type Header Extraction 已接入：新增 `EngineLifecycleSnapshot.h` 承载 `EngineSubsystemLifecycleSummary` / `EngineLifecycleSnapshot`；runtime engine verification report formatter 不再为了读取快照字段 include 完整 `Engine.h`。
- Engine Run Mode Header Extraction 已接入：新增 `EngineRunMode.h` 单独承载 `EngineRunMode`；`EngineLifecycleSnapshot.h` 不再为了 run mode 枚举 include 完整 `EngineContext.h`。
- Engine Desc Header Extraction 已接入：新增 `EngineDesc.h` 单独承载 `EngineDesc`；application config policy / engine startup bridge 不再为了构造启动 desc include 完整 `EngineContext.h`。
- Runtime Editor Lifecycle Config/State Header Split 已接入：新增 `RuntimeEditorLifecycleConfig.h` 与 `RuntimeEditorLifecycleState.h`，application state 不再为了 editor lifecycle state 间接包含完整 editor lifecycle 行为头和 frame callback 依赖。
- Runtime Application State Engine Owner Boundary Cleanup 已接入：新增 `RuntimeApplicationState.cpp`，application state header 只 forward declare `GLengine::Engine`，Engine owner 的完整 `Engine.h` 依赖局部化到 state implementation。
- Runtime Application State Legacy Runner Owner Boundary Cleanup 已接入：application state header 只 forward declare `LegacyExperimentRunner`，legacy runner owner 的完整 legacy experiment implementation 依赖局部化到 state implementation。
- Runtime Application State Runtime Context Owner Boundary Cleanup 已接入：application state header 只 forward declare `AppRuntimeContext`，runtime context owner 的完整 `AppRuntimeContext.h` 依赖局部化到 state implementation，并通过 `runtime()` accessor 供 startup/frame/shutdown bridge 使用。
- Runtime Application State Editor Lifecycle State Owner Boundary Cleanup 已接入：application state header 只 forward declare `RuntimeEditorLifecycleState`，editor lifecycle state 的完整 `RuntimeEditorLifecycleState.h` / `EditorPanels.h` 依赖局部化到 state implementation，并通过 `editorLifecycle()` accessor 供 frame editor callback bridge 使用。
- Runtime Application State Frame Lifecycle State Owner Boundary Cleanup 已接入：application state header 只 forward declare `RuntimeFrameLifecycleState`，frame lifecycle state 的完整 `RuntimeFrameLifecycleState.h` / `RuntimeFrameClock.h` / `<chrono>` 依赖局部化到 state implementation，并通过 `frameLifecycle()` accessor 供 frame startup/continue/run bridge 使用。
- Runtime Application State Engine Lifecycle State Accessor Boundary Cleanup 已接入：application state header 只 forward declare `RuntimeEngineLifecycleState`，engine lifecycle state 的完整依赖局部化到 state implementation 和 frame run 字段读取，shutdown cleanup refs 依赖局部化到 cleanup/report 路径，并通过 `engineLifecycle()` accessor 供 startup/content/frame/shutdown bridge 使用。
- Runtime Content Verification Sub-Lifecycle Facade Dependency Cleanup 已接入：`RuntimeContentVerificationLifecycle.h` 已确认只保留 forward declaration，implementation 不再依赖 generic `RuntimeVerificationLifecycle` facade，startup profile 与 prepared-scene report 分别直接委托到更小的 verification sub-lifecycle。
- Runtime Application Shutdown Verification Bridge Cleanup Lifecycle Direct Dependency Cleanup 已接入：shutdown verification bridge 不再依赖 generic `RuntimeVerificationLifecycle` facade，renderer subsystem cleanup 与 Engine cleanup report 直接委托 `RuntimeVerificationCleanupLifecycle`。
- Runtime Verification Lifecycle Facade Removal 已接入：frame lifecycle 的 stop/capture 路径直接依赖 `RuntimeVerificationStopPolicy` 与 `RuntimeVerificationFrameCaptureLifecycle`，无调用者的 `RuntimeVerificationLifecycle.h/.cpp` 已删除并从 VS project/filter 注册中移除。
- Runtime Application Editor Startup State Parameter and Callback Binder Include Cleanup 已接入：editor startup lifecycle 不再接收未使用的 `RuntimeApplicationState&`，callback binder implementation 不再 include 完整 config/state header，实际 state accessor 调用点仍显式保留 state header 依赖。
- Runtime Application Config Include Surface Follow-up Cleanup 已接入：frame editor callback bridge implementation 移除冗余完整 config include，runner public header 改为 forward declare shell config，完整 config include 保留在 runner implementation。
- Runtime Verification Args Public Header Config Forward Boundary Cleanup 已接入：verification args public header 不再传递完整 application config header，完整 config 依赖局部化到 args implementation 和 application entry 调用点。
- Runtime PBR Verification Args Compatibility Facade Removal 已接入：无外部引用的 `RuntimePBRVerificationArgs.h/.cpp` 兼容空壳已删除并从 VS project/filter 注册中移除，verification args 唯一入口收敛到 runtime 命名。
- Runtime Camera/Scene Prepare Config Consumer Header Forward Boundary Cleanup 已接入：`RuntimeCameraLifecycle`、`RuntimeLegacyExperimentLifecycle`、`RuntimeSceneSetupContextFactory` 与 `RuntimeSceneSetupPipelineLifecycle` public headers 不再传递完整 camera/scene prepare config，完整 DTO 依赖局部化到对应 implementation。
- Runtime Engine Lifecycle State/Cleanup Refs Header Split 已接入：旧 `RuntimeEngineLifecycleTypes.h` 拆为 `RuntimeEngineLifecycleState.h` 与 `RuntimeEngineLifecycleCleanupRefs.h`，frame/state 路径与 shutdown cleanup/report 路径不再共享同一个宽类型头。
- Runtime Content Config Policy extraction 已接入：`RuntimeApplicationShell` 不再直接组装 `RuntimeContentLifecycleConfig`，verification scene policy 到 content lifecycle config 的映射已迁入 `RuntimeContentConfigPolicy`。
- Runtime Frame Lifecycle extraction 已接入：`RuntimeApplicationShell` 不再直接管理 frame clock、rendered frame count、verification capture flag、frame runner 调用和 max-frame continue 条件，这些职责已迁入 `RuntimeFrameLifecycle`。
- Runtime Editor Lifecycle extraction 已接入：`RuntimeApplicationShell` 不再直接初始化 `RuntimeGuiHost` 或调用 `RuntimeEditorPanelCoordinator`，editor selection 与 transaction state 已迁入 `RuntimeEditorLifecycleState`。
- Runtime Graphics Lifecycle extraction 已接入：`RuntimeApplicationShell` 不再直接执行 viewport 初始化、clear color 设置或 OpenGL capability 输出，这些 startup graphics/diagnostics 细节已迁入 `RuntimeGraphicsLifecycle`。
- Engine Lifecycle Snapshot 已接入：`Engine::captureLifecycleSnapshot()` 提供 Engine-owned lifecycle 快照，Editor diagnostics 与 verification 共用该快照观察 initialized/run mode/viewport/tick/subsystem/active World 状态。
- Runtime Application Config Policy extraction 已接入：`RuntimeApplicationShell` 不再直接组装 Engine desc、frame lifecycle config、editor lifecycle config 或 graphics lifecycle config，这些非 content config 映射已迁入 `RuntimeApplicationConfigPolicy`。
- Runtime Window Lifecycle Snapshot Boundary 已接入：`RuntimeApplicationShell` 不再直接读取 `Application` 单例的 width/height/window 或 destroy，当前窗口快照和销毁入口已集中到 `RuntimeWindowLifecycle`。
- Engine Subsystem Lifecycle Summary Snapshot 已接入：`EngineSubsystem` 提供通用只读诊断接口，`Engine::captureLifecycleSnapshot()` 现在可枚举 Engine-owned subsystem 的 name、initialized 和 tick count，diagnostics 与 verification 共用该 summary。
- Runtime Application Startup Lifecycle extraction 已接入：`RuntimeApplicationShell::initialize()` 不再直接展开 Engine / Window / Graphics / Content / Editor / Frame reset 启动顺序，startup 编排已迁入 `RuntimeApplicationStartupLifecycle`。
- Runtime Application Shutdown Lifecycle extraction 已接入：`RuntimeApplicationShell::cleanup()` / `destroy()` 不再直接展开 renderer cleanup report、camera cleanup、runtime context detach、Engine shutdown、Engine cleanup report 或 window destroy，shutdown 编排已迁入 `RuntimeApplicationShutdownLifecycle`。
- Runtime Application Frame Lifecycle extraction 已接入：`RuntimeApplicationShell::shouldContinue()` / `runFrame()` 不再直接组装 frame config、window snapshot、editor callbacks 或调用 `RuntimeFrameLifecycle`，frame 编排已迁入 `RuntimeApplicationFrameLifecycle`。
- Runtime Application State Context extraction 已接入：`RuntimeApplicationShell` 不再分散持有 Engine、runtime context、engine/editor/frame lifecycle state 和 legacy experiments，而是通过 `RuntimeApplicationState` 统一持有 application runtime state。
- Engine diagnostics panel first slice 已接入：`EngineDiagnosticsPanel` 接管 RendererSubsystem frame bridge diagnostics 绘制，DebugControllerPanel 不再直接读取 RendererSubsystem stats。
- Engine diagnostics unified context 已接入：`AppRuntimeContext` 暴露非拥有 Engine 指针，Engine diagnostics panel 同时显示 Engine runtime、World、AssetSubsystem 和 RendererSubsystem 基础状态；verification 断言 `runtimeContextEngineAttached=yes`。
- Engine-driven subsystem health counters 已接入：Engine / World / AssetSubsystem / RendererSubsystem 都记录 tick count，Engine diagnostics panel 显示统一 health stats，verification 断言四个 tick counter 大于 0 且相等，并确认 context World 匹配 active World。
- `tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures` 已通过当前默认 34 个 PBR verification mode，包含 Engine lifecycle snapshot、`engine-world-scene-package` 与 `renderer-backend-registry-noop` mode。

## 下一阶段

当前最新修正：SceneSetup Context Header Boundary Cleanup 已接入后，`SceneSetup.h` 不再传播 scene/light/material/mesh/Bloom/environment/frame-target/renderer/profile 完整 headers；`SetupContext` public header 收敛为引用/shared_ptr 契约和前置声明，完整依赖集中到 scene setup implementation、pipeline implementation 和 runtime lifecycle complete-context 调用点；下一步继续 runtime/scene setup 或 editor public header include audit，但不扩张 PBR pass。

下一阶段建议继续推进 Engine runtime ownership：Engine runtime ownership 已完成多轮 boundary/header extraction。本轮 scene setup context header cleanup 后，下一步优先继续 runtime/scene setup、editor public header 或 Engine public header 低风险 include audit；当前不建议继续扩张 PBR pass。
