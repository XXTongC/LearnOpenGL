# Subagents Coordination

## Purpose

This file defines the coordination boundary for multi-agent work on the `text2-refactor` engine transformation. The parent Codex agent remains responsible for final decisions, integration, verification, and `worked.md` updates.

## Subagent Restart Protocol

- The user has authorized considering subagents for continued goal work, but the parent agent must update this shared boundary document before spawning or resuming sidecar work.
- The active goal is continued, not recreated, when `get_goal` reports an active objective; this avoids resetting long-running refactor history.
- Default sidecar mode is read-only audit. A subagent may edit files only when the parent assigns an explicit disjoint write scope before spawning.
- Parent-owned files for each round must be recorded in this document before work starts; sidecar-owned files must be disjoint from parent-owned files.
- `imgui.ini` remains unrelated local state and must not be edited by parent or sidecar agents unless the user explicitly asks.
- Sidecar findings are advisory until the parent verifies, integrates, and records accepted work in `worked.md`.
- Subagents must report using the shared sidecar communication format at the end of this document.

## Current Goal

Continue the engine refactor according to the project plan, with self-directed implementation and verification.

Current phase:

- Engine owns the active runtime `World`.
- Engine owns `AssetSubsystem`, and `AppRuntimeContext` only keeps a non-owning pointer.
- `Engine::tick(...)` is called every runtime frame before the legacy renderer pipeline.
- Runtime frame delta is now produced by `RuntimeFrameClock`, with fixed delta retained for verification.
- Engine owns `RendererSubsystem`, and the legacy runtime renderer is attached as a non-owning runtime renderer pointer.
- Runtime frames now enter rendering through `RendererSubsystem::renderFrameBridge(...)`, which records frame intent, executor attachment, neutral frame pass counts, and a stable frame plan key without taking ownership of the application renderer.
- `RuntimeRendererFrameBridgeAdapter` implements the engine-side `RendererBackend` contract and owns the translation from the legacy `RuntimeFramePipeline` result to engine-side neutral frame result fields.
- `RuntimeFramePipeline.h` and `RuntimeFramePasses.h` now forward declare `AppRuntimeContext`; complete runtime context dependency is kept in frame pipeline/pass implementations that read context fields.
- `RuntimeFramePassRegistry.h` uses `std::string_view` for pass key lookup, so registry key queries no longer force public `<string>` propagation.
- `FrameRenderTargets.h` now forward-declares `Framebuffer` and `Texture`; complete framebuffer/texture implementation dependencies are localized to `FrameRenderTargets.cpp`.
- `PostProcessPass.h` now forward-declares framebuffer, mesh, shader, bloom, and post-process settings types; complete render implementation dependencies are localized to `PostProcessPass.cpp`.
- `RendererBackend` exposes a stable backend key and backend readiness state before frames are executed; the current runtime adapter checks readiness against the enabled frame pass dependencies.
- `RendererSubsystem` now records backend lifecycle stats: backend state, attach/detach count, and ready/not-ready frame counts.
- `RendererBackend.h` now owns only the renderer backend interface contract and forward declares `RendererFrameIntent` / `RendererFrameResult`; complete frame DTO definitions live in `RendererBackendFrameTypes.h` and are included by concrete backend implementations that read or construct them.
- `RendererBackendRegistryTypes.h` owns renderer backend registry metadata: `RendererBackendAttachmentDesc`, `RendererBackendRegistration`, and `RendererBackendSelection`.
- Renderer backend registry/catalog/factory metadata paths no longer need the full `RendererBackend.h` interface header unless they create, own, or invoke a concrete backend.
- `RuntimeRendererBackendKeys.h` owns lightweight runtime/default/no-op backend key helpers; application config default construction, verification args, catalog, and factory use it locally without making `RuntimeApplicationConfig.h` propagate the helper.
- `RuntimeWindowLifecycleTypes.h` owns window lifecycle DTOs (`RuntimeWindowConfig`, `RuntimeWindowSnapshot`, and `RuntimeWindowCallbackContext`), so application config and snapshot-only startup code do not need the full window lifecycle behavior header or `AppRuntimeContext.h`.
- `RuntimeFrameClockTypes.h` owns `RuntimeFrameClockConfig`, so application config does not need the full frame clock behavior header or `<chrono>` just to store frame timing options.
- `RuntimeFrameLifecycleConfig.h` owns frame lifecycle config separately from `RuntimeFrameLifecycleState.h`, and `RuntimeFrameLifecycleState.h` now hides `RuntimeFrameClock` behind an implementation owner; config/state users no longer receive the clock behavior header or `<chrono>` through these lifecycle DTO boundaries.
- `RuntimeEditorLifecycleConfig.h` and `RuntimeEditorLifecycleState.h` split editor lifecycle data from behavior, so application state and config policy do not need the full editor lifecycle behavior header just to store editor state or config.
- `RuntimeApplicationState.cpp` now owns construction/destruction of the application Engine; `RuntimeApplicationState.h` forward-declares `GLengine::Engine` and exposes an `engine()` accessor instead of including the full `Engine.h` header.
- `RuntimeApplicationState.cpp` now also owns construction/destruction of the legacy experiment runner; `RuntimeApplicationState.h` forward-declares `GL_EXPERIMENTS::LegacyExperimentRunner` and exposes a `legacyExperiments()` accessor instead of including the full legacy experiment implementation header.
- `RuntimeApplicationState.cpp` now also owns construction/destruction of the application runtime context; `RuntimeApplicationState.h` forward-declares `GLframework::AppRuntimeContext` and exposes a `runtime()` accessor instead of including the full `AppRuntimeContext.h` header.
- `RuntimeApplicationState.cpp` now also owns construction/destruction of the editor lifecycle state; `RuntimeApplicationState.h` forward-declares `RuntimeEditorLifecycleState` and exposes an `editorLifecycle()` accessor instead of including `RuntimeEditorLifecycleState.h` and indirectly `EditorPanels.h`.
- `RuntimeApplicationState.cpp` now also owns construction/destruction of the frame lifecycle state; `RuntimeApplicationState.h` forward-declares `RuntimeFrameLifecycleState` and exposes a `frameLifecycle()` accessor instead of including `RuntimeFrameLifecycleState.h`, `RuntimeFrameClock.h`, and `<chrono>`.
- `RuntimeApplicationState.cpp` now also owns construction/destruction of the engine lifecycle state; `RuntimeApplicationState.h` forward-declares `RuntimeEngineLifecycleState` and exposes an `engineLifecycle()` accessor instead of including the full engine lifecycle state header.
- `RendererSubsystem` now owns the active runtime renderer backend through `std::unique_ptr<RendererBackend>`; `RuntimeApplicationShell` creates the backend through `RuntimeRendererBackendFactory` and transfers ownership into the Engine-owned subsystem.
- `RuntimeRendererBackendFactory` now exposes a key-based registry so `RuntimeApplicationShellConfig::rendererBackendKey` selects the runtime renderer backend instead of calling a hard-coded creation path.
- The backend registry now has a dedicated no-op/test backend that verifies backend selection without invoking the legacy `RuntimeFramePipeline`.
- Renderer backend key lookup/attachment APIs now accept `std::string_view` across content, Engine lifecycle, attachment lifecycle, runtime backend catalog, and Engine backend registry boundaries; config/selection/attachment descriptors still store stable `std::string` keys.
- Verification now checks renderer backend cleanup: the Engine-owned `RendererSubsystem` must detach and clear backend metadata before runtime context pointers are cleared.
- Verification is being neutralized around a renderer backend contract surface: existing subsystem/PBR-oriented output remains for compatibility, while new `Runtime renderer backend contract ...` lines describe backend attachment, readiness, frame pass counts, no-op backend behavior, and cleanup state without implying a PBR-only renderer boundary.
- Verification now checks Engine World cleanup: after runtime context raw pointers are cleared and `Engine::shutdown()` runs, the Engine must be uninitialized, active World must be reset, and runtime context Engine / World / subsystem pointers must be null.
- Verification now checks Engine subsystem cleanup: after `Engine::shutdown()`, AssetSubsystem registry/ticks must reset and RendererSubsystem must release renderer/executor/backend state while preserving attach/detach lifecycle evidence.
- `World.h` now forward-declares `Level` and owns the persistent level through an out-of-line destructor, so code that only needs the World facade no longer receives the full Level/Actor template surface.
- `Actor.h` now forward-declares `SceneComponent` for root-component pointer APIs, so code that only needs the Actor facade no longer receives the full SceneComponent/Transform surface.
- `Actor.h` now also forward-declares `ActorComponent` and keeps Actor destruction out-of-line, so code that only needs the Actor facade no longer receives the full ActorComponent lifecycle API surface.
- `RuntimeEngineLifecycle` now owns application-side Engine lifecycle composition: creating Engine-owned subsystems, attaching renderer backends, detaching runtime context pointers, and shutting down the Engine.
- `RuntimeEngineLifecycleState.h` now owns `RuntimeEngineLifecycleState`, and `RuntimeEngineLifecycleCleanupRefs.h` now owns `RuntimeEngineLifecycleCleanupRefs`, so frame/state paths and shutdown cleanup/report paths do not share one broad lifecycle types header.
- `RuntimeEngineLifecycle.h` now forward-declares lifecycle state/cleanup refs; full state/cleanup refs dependencies are localized to implementation, state owners, frame field access, and cleanup refs field access sites.
- `EngineLifecycleSnapshot.h` now owns `EngineSubsystemLifecycleSummary` and `EngineLifecycleSnapshot`, so runtime engine verification report formatting can read snapshot data without including the full `Engine.h` owner header.
- `EngineRunMode.h` now owns `EngineRunMode`, so lifecycle snapshot/report paths can depend on the run-mode enum without including the full `EngineContext.h` desc/context data model.
- `EngineDesc.h` now owns `EngineDesc`, so application startup/config policy paths can construct engine startup parameters without including the full per-frame `EngineContext.h`.
- `Engine.h` now forward-declares `EngineLifecycleSnapshot`; complete lifecycle snapshot dependencies are localized to `Engine.cpp` and diagnostics/reporting translation units that read snapshot fields.
- `Engine.h` no longer exposes full `EngineContext.h` or `World.h`; it forward-declares `EngineContext`, `EngineDesc`, and `World`, while `Engine.cpp` owns the complete context/world dependencies and `EngineContext` storage.
- `Engine.h` now also forward-declares `EngineSubsystem`; complete subsystem lifecycle and diagnostics API dependencies are localized to `Engine.cpp` and concrete subsystem headers.
- `Engine::addSubsystem(...)` no longer dereferences `mContext` inside the public template body; initialized-subsystem context handoff is routed through an out-of-line `Engine.cpp` helper.
- `RuntimeApplicationShutdownCleanupBridge` now receives `RuntimeVerificationConfig` directly; the shell-config-to-verification-config mapping is localized to `RuntimeApplicationShutdownLifecycle.cpp`.
- `RuntimeApplicationShutdownLifecycle` now receives `RuntimeVerificationConfig` directly; `RuntimeApplicationCallbackBinder` owns the shell-config-to-verification-config mapping for cleanup callbacks.
- `RuntimeApplicationShutdownCallbackBridge` now owns shutdown callback shell-config mapping and lifecycle forwarding, so `RuntimeApplicationCallbackBinder` no longer directly includes shutdown lifecycle or config policy details.
- `RuntimeApplicationFrameCallbackBridge` now owns frame callback shouldContinue/runFrame forwarding, so `RuntimeApplicationCallbackBinder` no longer directly includes `RuntimeApplicationFrameLifecycle.h`.
- `RuntimeApplicationStartupCallbackBridge` now owns startup callback initialize forwarding, so `RuntimeApplicationCallbackBinder` no longer directly includes `RuntimeApplicationStartupLifecycle.h`.
- `RuntimeBootstrapperCallbacks.h` now owns the bootstrapper callback DTO, so callback construction/return paths do not need the full `RuntimeBootstrapper.h` runner facade.
- `RendererSubsystem.h` now hides backend slot, frame execution bridge, and frame bridge state implementation members behind private owning pointers instead of exposing internal implementation headers.
- The old `RuntimeVerificationLifecycle` facade has been removed; startup profile, prepared-scene reporting, frame capture/reporting, cleanup reporting, and max-frame stop checks now route through narrower verification sub-lifecycle/policy modules.
- `RuntimeContentLifecycle` now owns application-side startup content composition: camera initialization, runtime profile load, verification startup profile, scene preparation, renderer backend lifecycle attachment, and prepared-scene reporting.
- `RuntimeCameraConfig.h`, `RuntimeScenePrepareConfig.h`, and `RuntimeContentLifecycleConfig.h` now own content startup config DTOs; `RuntimeContentLifecycle.h` now forward-declares the aggregate content config instead of exposing the full config DTO header.
- `RuntimeContentRendererBackendLifecycle` now owns the post-scene renderer fail-fast gate and Engine-owned renderer backend attachment after scene preparation, so generic content composition no longer directly logs this failure or calls `RuntimeEngineLifecycle::attachRendererBackend(...)`.
- `RuntimeContentVerificationLifecycle` now owns runtime profile loading, verification startup profile application, and prepared-scene verification reporting for content startup, so generic content composition no longer directly calls `RuntimeProfileLoader` or `RuntimeVerificationLifecycle`.
- `RuntimeContentVerificationLifecycle.cpp` no longer depends on the generic `RuntimeVerificationLifecycle` facade for startup profile or prepared-scene reporting; it now calls the narrower verification sub-lifecycle modules directly.
- `RuntimeContentSceneLifecycle` now owns the content-side scene preparation stage, so generic content composition no longer directly calls `RuntimeScenePreparer`.
- `RuntimeContentCameraLifecycle` now owns startup camera initialization for content startup, so generic content composition no longer directly calls `RuntimeCameraLifecycle`.
- `RuntimeContentCameraLifecycle.h` and `RuntimeContentSceneLifecycle.h` now forward-declare their config DTOs; full camera/scene config dependencies are localized to their implementations and the aggregate content lifecycle config.
- Application shutdown cleanup refs now use the canonical `RuntimeEngineLifecycleCleanupRefs`; the application shutdown bridge no longer owns a duplicate cleanup refs DTO.
- `RuntimeApplicationShutdownVerificationBridge` now consumes `RuntimeVerificationConfig` instead of the full shell config, so shutdown cleanup reporting no longer needs the complete `RuntimeApplicationConfig.h` surface.
- `RuntimeApplicationContentStartupLifecycle` now owns the application startup bridge from window snapshot and shell config into content config policy plus `RuntimeContentLifecycle::prepare(...)`, so `RuntimeApplicationStartupLifecycle` no longer directly depends on content config policy or content lifecycle.
- `RuntimeApplicationEditorStartupLifecycle` now owns the application startup bridge from window snapshot and shell config into editor lifecycle config plus `RuntimeEditorLifecycle::initialize(...)`, so `RuntimeApplicationStartupLifecycle` no longer directly depends on editor lifecycle.
- `RuntimeApplicationFrameStartupLifecycle` now owns the application startup bridge into `RuntimeFrameLifecycle::reset(...)`, so `RuntimeApplicationStartupLifecycle` no longer directly resets frame lifecycle.
- `RuntimeApplicationEngineStartupLifecycle` now owns the application startup bridge from shell config into Engine desc mapping plus `RuntimeEngineLifecycle::initializeEngine(...)`, so `RuntimeApplicationStartupLifecycle` no longer directly initializes Engine lifecycle.
- `RuntimeApplicationWindowStartupLifecycle` now owns the application startup bridge into window setup prompt, window initialization, and window snapshot capture, so `RuntimeApplicationStartupLifecycle` no longer directly initializes or reads window lifecycle.
- `RuntimeApplicationGraphicsStartupLifecycle` now owns the application startup bridge from shell config into graphics config mapping plus `RuntimeGraphicsLifecycle::initializeAfterWindow(...)`, so `RuntimeApplicationStartupLifecycle` no longer directly initializes graphics lifecycle.
- `RuntimeApplicationStartupLifecycle.cpp` now only depends on startup bridge headers; redundant full config/state includes have been removed from startup facade/content/editor/engine startup implementation files where not needed, and `RuntimeApplicationGraphicsStartupLifecycle.h` now forward-declares shell config.
- `RuntimeApplicationFrameEditorCallbackBridge` now owns the application frame bridge from shell config/window handle into editor frame callback config plus `RuntimeEditorLifecycle::makeFrameCallbacks(...)`, so `RuntimeApplicationFrameLifecycle` no longer directly creates editor frame callbacks.
- `RuntimeApplicationFrameEditorCallbackBridge.h` now forward-declares `RuntimeFrameCallbacks`; the full callback DTO dependency is localized to `RuntimeApplicationFrameEditorCallbackBridge.cpp`.
- `RuntimeApplicationFrameContinueBridge` and `RuntimeApplicationFrameRunBridge` now own application frame continue/run implementation details, so `RuntimeApplicationFrameLifecycle` is reduced to a facade over frame continue and run calls.
- `RuntimeApplicationFrameLifecycle.cpp` now only depends on frame continue/run bridge headers; redundant full config/state includes have been removed from the frame facade and bridge implementation files where not needed.
- `RuntimeApplicationShutdownCleanupBridge` and `RuntimeApplicationShutdownDestroyBridge` now own application shutdown cleanup/destroy implementation details, so `RuntimeApplicationShutdownLifecycle` is reduced to a facade over shutdown cleanup and destroy calls.
- `RuntimeApplicationShutdownVerificationBridge` now owns shutdown cleanup verification report delegation, so `RuntimeApplicationShutdownCleanupBridge` no longer directly depends on `RuntimeVerificationLifecycle`.
- `RuntimeApplicationShutdownEngineBridge` now owns shutdown begin cleanup, camera cleanup, runtime context detach, and Engine shutdown delegation, so `RuntimeApplicationShutdownCleanupBridge` no longer directly depends on `RuntimeCameraLifecycle` or `RuntimeEngineLifecycle`.
- `RuntimeApplicationShutdownLifecycle.cpp` now only depends on shutdown cleanup/destroy bridge headers; redundant full config/state includes have been removed from shutdown facade and cleanup bridge implementation files where not needed.
- `RuntimeLegacyExperimentLifecycle` now owns legacy experiment runtime context construction, startup enable hooks, and per-frame update, so `RuntimeFrameRunner` no longer depends on `RuntimeScenePreparer` for legacy experiment ticking.
- `SceneSetupPipelineConfig.h` now owns the lightweight scene setup pipeline config DTO, so `RuntimeScenePrepareConfig.h` no longer exposes the full `SceneSetupPipeline.h` API surface.
- `LegacySceneWorldStats.h` now owns legacy scene import/export stats DTOs, so scene setup pipeline result headers no longer expose full legacy world builder/exporter behavior APIs just to store stats.
- `WorldDrivenSceneStats.h` now owns world-driven scene probe/minimal-scene result DTOs, so `SceneSetupPipeline.h` no longer exposes full `WorldDrivenSceneSetup.h` behavior APIs just to store result stats.
- `PBRLightRigProfile.h` now forwards runtime light parameter types and includes only explicit `glm` value types; full light behavior headers are localized to the light rig implementation that creates and reads lights.
- `PBRExperimentProfile.h` now forwards environment/postprocess/preview/light/camera profile parameter types; full profile definitions and config/property builder dependencies are localized to the experiment profile implementation.
- `PBRMaterialProfile.h` now owns material profile/storage DTOs, so `PBRPreviewProfile.h` no longer exposes full runtime `PBRMaterial.h` behavior APIs just to store a by-value material preset.
- `AssimpMaterialImporter.h`, `assimpLoader.h`, and `assimpInstanceLoader.h` now expose only import public contracts and forward declarations; full material/Assimp/mesh/renderer/texture/shader import helper dependencies are localized to implementation files.
- `MaterialTypes.h` now owns `MaterialType` and `PreStencilType`, so `ShaderLibrary.h` no longer exposes full `material.h` behavior APIs just to store/query material shader keys.
- `LegacyExperimentRunner` is no longer header-only; legacy experiment construction/update logic now lives in `LegacyExperimentRunner.cpp`, while the public header exposes only runtime context/API declarations and forward declarations for legacy runtime types.
- `RuntimeSceneSetupReport` now owns scene setup result stdout/logger reporting and the renderer prepared log line, so `RuntimeScenePreparer` no longer directly depends on logger/stdout or scene setup stats formatters.
- `RuntimeSceneSetupContextFactory` now owns the `AppRuntimeContext` to `GL_SCENE::SetupContext` field mapping, so `RuntimeScenePreparer` no longer exposes or implements `makeSceneSetupContext(...)`.
- `RuntimeSceneSetupPipelineLifecycle` now owns setup context creation, scene setup pipeline execution, and prepared-scene setup reporting, so `RuntimeScenePreparer` no longer directly includes full `SceneSetupPipeline.h`.
- `RuntimeContentConfigPolicy` now owns the mapping from `RuntimeApplicationShellConfig` to `RuntimeContentLifecycleConfig`, including verification scene flags to scene setup pipeline policy.
- `RuntimeContentConfigPolicy.h` now exposes only forward declarations; full shell config and content lifecycle config dependencies are localized to `RuntimeContentConfigPolicy.cpp` and the content startup call site that consumes the returned config temporary.
- `RuntimeContentLifecycle.h` now forward-declares `RuntimeContentLifecycleConfig`; full config DTO access is localized to `RuntimeContentLifecycle.cpp` and `RuntimeApplicationContentStartupLifecycle.cpp`.
- `RuntimeFrameLifecycle` now owns application-side frame lifecycle composition: max-frame continue checks, frame clock state, runtime frame runner invocation, rendered frame count, GUI callback gating, and verification capture state.
- `RuntimeFrameLifecycleConfig.h` and `RuntimeFrameLifecycleState.h` now own frame lifecycle config/state separately; the old compatibility aggregator `RuntimeFrameLifecycleTypes.h` has been removed, so callers use the narrow canonical header they actually need.
- `RuntimeFrameRunnerTypes.h` now owns `RuntimeFrameConfig`, so `RuntimeFrameRunner.h` exposes only the runner facade, callback DTO forward declaration, and a config forward declaration.
- `RuntimeFrameRunner.h` and `RuntimeFrameLifecycle.h` now expose no-callback overloads instead of `RuntimeFrameCallbacks` default arguments, so complete callback DTO dependencies stay in implementation files and the frame run bridge that constructs editor callbacks.
- `RuntimeApplicationShell.h` now hides `RuntimeApplicationShellConfig` behind a private owning pointer, so shell users do not include the full application config aggregate just to hold or invoke the shell facade.
- `RuntimeEditorLifecycle` now owns application-side editor lifecycle composition: GUI host initialization, editor panel frame callback creation, selection state, and edit transaction state.
- `RuntimeEditorLifecycle.h` now forward-declares `RuntimeFrameCallbacks`; the full callback DTO dependency is localized to `RuntimeEditorLifecycle.cpp` and frame editor callback bridge implementation paths that construct or consume the complete value.
- `RuntimeGraphicsLifecycle` now owns application-side startup graphics composition: window setup prompt, viewport initialization, clear color setup, and OpenGL capability diagnostics.
- `RuntimeApplicationConfigPolicy` now owns non-content shell config mapping: Engine desc, frame lifecycle config, editor lifecycle config, and graphics lifecycle config.
- `RuntimeApplicationConfigPolicy.h` now exposes only forward declarations; full shell config, lifecycle config DTO, and EngineContext dependencies are localized to `RuntimeApplicationConfigPolicy.cpp` and the call sites that require complete return types.
- `RuntimeWindowLifecycle` now owns the current window snapshot and destroy entry so `RuntimeApplicationShell` no longer directly reads `Application` singleton width/height/window state.
- `RuntimeApplicationStartupLifecycle` now owns application-side startup orchestration, so `RuntimeApplicationShell::initialize()` only delegates the startup sequence.
- `RuntimeApplicationFrameLifecycle` now owns application-side frame orchestration, so `RuntimeApplicationShell::shouldContinue()` and `RuntimeApplicationShell::runFrame()` only delegate frame sequencing.
- `RuntimeApplicationShutdownLifecycle` now owns application-side cleanup/destroy orchestration, so `RuntimeApplicationShell::cleanup()` and `RuntimeApplicationShell::destroy()` only delegate shutdown sequencing.
- `RuntimeApplicationState` now owns the application-side runtime state aggregate, so `RuntimeApplicationShell` only holds state plus config instead of individual Engine/runtime/editor/frame members.
- `RuntimeApplicationShell.h` now hides `RuntimeApplicationState` behind `std::unique_ptr`, and application startup/frame/shutdown bridge public headers forward-declare state/config instead of including the full application state/config headers.
- `RuntimeApplicationCallbackBinder.h` now forward-declares `RuntimeBootstrapperCallbacks`; complete bootstrapper callback DTO access is localized to callback binder and shell implementations.
- `RuntimeApplicationWindowStartupLifecycle.h` now forward-declares `RuntimeWindowSnapshot`; complete window snapshot access is localized to startup/window implementation files that construct or consume the snapshot.
- PBR verification profile/prepared-scene stats implementations no longer include the full `RuntimeVerificationConfig.h` when they only forward or ignore the const verification config reference; full config dependencies stay in modules that read config fields.
- `RuntimeProfileState.h` now owns runtime profile/path data for frame pipeline, post-process, environment, and PBR preview/light/camera profiles; `AppRuntimeContext` exposes these through `context.profiles` instead of top-level profile fields.
- `RuntimeRenderResourceState.h` now owns AppRuntimeContext render resources: renderer, screen/world scenes, runtime meshes/materials, frame render targets, bloom, post-process pass, and clear color; application code accesses these through `context.renderResources`.
- `RuntimeCameraLightState.h` now owns AppRuntimeContext camera/control and ambient/directional/spot/point light state; application code accesses these through `context.cameraLights`.
- `RuntimeEngineAttachmentState.h` now owns AppRuntimeContext Engine, World, AssetSubsystem, RendererSubsystem, and engine-world editable attachment state; application code accesses these through `context.engineAttachments`.
- `RuntimeRendererBackendAttachmentLifecycle` now owns runtime renderer backend selection, factory creation, attachment desc creation, and `RendererSubsystem::setRendererBackend(...)`; `RuntimeEngineLifecycle` delegates backend attachment instead of knowing catalog/factory details.
- `RendererSubsystemFrameBridgeStats.h` now owns the frame bridge stats DTO, so `RendererSubsystem.h` no longer carries the stats field list or direct `<string>` dependency.
- `RendererSubsystemFrameBridgeState` now owns frame bridge stats mutation policy: frame intent/result application, backend metadata refresh, frame counters, and Engine time/delta refresh.
- `RendererSubsystemBackendSlot` now owns renderer backend pointer storage, attachment metadata normalization, backend readiness checks, and attach/detach change detection; `RendererSubsystem` keeps lifecycle and frame dispatch facade responsibilities.
- `RendererSubsystemBackendSlotSnapshot` now carries backend observation data from the backend slot into `RendererSubsystemFrameBridgeState`, so frame bridge stats no longer probe raw `RendererBackend*` for readiness or backend key.
- `RendererSubsystemFrameExecutionBridge` now owns backend frame execution and default frame result generation; `RendererSubsystem` keeps subsystem lifecycle, begin/end frame bridge, and stats counter coordination.
- `RendererSubsystem` now exposes only renderer backend API names (`setRendererBackend`, `clearRendererBackend`, `hasRendererBackend`) for live application code; old `FrameExecutor` public wrappers and renderer DTO aliases have been removed.
- `Engine::captureLifecycleSnapshot()` now provides an Engine-owned lifecycle snapshot shared by Editor diagnostics and verification.
- `Engine::captureLifecycleSnapshot()` now also includes Engine-owned subsystem summaries with diagnostic name, initialized state, and tick count.
- `RendererFrameIntent` and `RendererFrameResult` define the current renderer backend DTO boundary; the old `RendererSubsystemFrameIntent` / `RendererSubsystemFrameResult` compatibility aliases have been removed.
- `EngineDiagnosticsPanel` owns the current RendererSubsystem frame bridge diagnostics UI instead of keeping that code inside the generic debug controller.
- `EngineDiagnosticsPanel` now also receives Engine, World, and AssetSubsystem context from `AppRuntimeContext`.
- Engine / World / AssetSubsystem / RendererSubsystem now expose tick counters, and verification checks that Engine tick drives the active World and Engine-owned subsystems together.
- `RuntimeVerificationReport` now owns generic runtime / Engine / renderer backend verification report output.
- `RuntimeEngineWorldVerification` now owns Engine World prepared-scene verification output: editor-create probe, transform snapshot save/apply, scene package round-trip, scene package negative probes, and runtime package resolver fixtures.
- `RuntimeEngineWorldVerification` now owns neutral Engine World prepared-scene counters through `Engine world prepared scene stats`, while `RuntimePBRVerification` keeps only PBR/render-scene-oriented scene counters.
- `RuntimeImportedAssetVerification` now owns imported asset Engine World import, AssetRegistry stats, and imported asset scene package manifest verification, while preserving the existing `--verify-pbr-import` output contract.
- `RuntimeVerificationCapture` now owns default framebuffer readback and PPM capture output, so verification lifecycle no longer calls through `RuntimePBRVerification` for generic capture.
- `RuntimePBRRendererStatsVerification` now owns capture-frame `PBR verification renderer stats` output, while keeping the existing verification output contract stable.
- `RuntimePBRProfileVerification` now coordinates PBR verification startup profile application and delegates startup defaults, pass profile, profile line, preview, and light/camera rig policy to dedicated modules.
- `RuntimePBRSceneProbeVerification` now owns PBR verification scene probe construction: transparent fallback, deferred emissive, material IBL, alpha mask, texture set, and showcase spheres.
- `RuntimePBRPreparedSceneStatsVerification` now owns `PBR verification scene stats`; the old `RuntimePBRVerification` class, `.cpp`, and `.h` have been removed from live code and project registration.
- `RuntimePBRVerificationConfig.h` now owns the PBR-specific verification config struct consumed by `RuntimeVerificationConfig`.
- `RuntimePBRVerificationConfig` is now grouped into pass, probe, and deferred override subconfigs, so args/profile/probe/import verification no longer depend on one flat PBR config field list.
- `RuntimePBRPassProfileVerification` now owns renderer pass profile writes for forward/deferred/G-buffer/debug/tiled/clustered/GPU timing verification modes.
- `RuntimePBRProfileLineVerification` now owns the `PBR verification profile applied` summary line assembly and output; `RuntimePBRProfileVerification` no longer depends on logger/stdout reporting.
- `RuntimePBRPreviewProfileVerification` now owns verification preview grid policy, including default material grid, showcase override, and minimal-scene disable behavior.
- `RuntimePBRLightCameraRigVerification` now owns PBR verification light/camera rig policy, including default, minimal-scene, tiled-light, showcase-camera, and pressure-light behavior.
- `RuntimePBRStartupProfileVerification` now owns PBR verification startup defaults for environment, post-process, and runtime frame pipeline profile.
- Next architectural direction remains Engine runtime ownership: keep old `frameExecutor*` verification fields as compatibility-only output until intentionally deprecated, or choose the next small Engine runtime boundary without expanding PBR scope.

## Coordination Rules

- Agents are not alone in the codebase. They must not revert or overwrite changes they did not make.
- Agents must treat `imgui.ini` as unrelated local state unless explicitly assigned.
- Agents must not run destructive git commands.
- Agents must not amend commits or reset the worktree.
- Agents must keep write scopes disjoint when asked to edit.
- Parent agent owns final merge, test execution, documentation consistency, and user-facing summary.
- If an agent finds a conflict with existing work, it reports the conflict instead of trying to force a replacement.

## Shared Communication Format

Each agent final report should include:

- Scope inspected.
- Files changed, if any.
- Findings or implementation summary.
- Verification performed.
- Risks or follow-up work.

For read-only exploration, agents must explicitly state that no files were changed.
For this project, each active round must also record the parent-owned write scope, any delegated write scope, and whether the delegated work is blocking or advisory.
If a delegated report recommends a change, the parent agent decides whether to integrate it and records the accepted operation in `worked.md`.

## Agent Boundaries

### Current Round: Runtime Render Resource PBR Scene Probe Boundary Cleanup

Parent mode: implementation owner.

Parent write scope:

- `application/RuntimeRenderResourceState.h`
- `application/RuntimeRenderResourceState.cpp`
- `application/RuntimePBRSceneProbeVerification.cpp`
- `docs/subagents_coordination.md`
- `work.md`
- `worked.md`
- related architecture docs after verification

Delegated mode: read-only advisory.

Delegated scope:

- none. This slice is parent-owned and does not start a new sidecar.
- Runtime render resource PBR scene probe boundary cleanup remains parent-reviewed.

Rules for this round:

- Subagents must not edit files.
- Subagents must not run build/test commands that write outputs unless the parent explicitly asks.
- Subagents must not run destructive git commands.
- Subagents must not touch `imgui.ini`.
- Subagents report findings only; parent decides and records accepted operations in `worked.md`.

### Resolver Audit Agent

Mode: read-only.

Scope:

- `engine/ScenePackage.h`
- `engine/ScenePackage.cpp`
- `application/RuntimeEngineWorldVerification.cpp`
- `application/RuntimeImportedAssetVerification.cpp`
- `tools/verify_pbr.ps1`
- related scene package docs

Task:

- Audit the current scene package resolver and verification coverage.
- Identify correctness issues, missing assertions, or architecture risks.
- Do not edit files.

### AssetRegistry Design Agent

Mode: read-only unless explicitly reassigned.

Scope:

- engine asset/package boundaries
- renderer material/texture/mesh ownership
- import pipeline references
- project docs

Task:

- Propose the smallest next AssetRegistry slice that advances the engine goal without expanding PBR.
- Identify likely files/classes to add and test gates.
- Do not edit files unless reassigned by the parent agent.

### Editor Asset Visibility Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/AppRuntimeContext.h`
- `application/RuntimeEditorPanelCoordinator.*`
- `tools/editor/EditorPanels.*`
- `engine/AssetRegistry.*`
- `application/RuntimeImportedAssetVerification.*`
- `application/RuntimePBRPreparedSceneStatsVerification.*`
- `tools/verify_pbr.ps1`

Task:

- Audit how imported asset handles can be exposed to the editor without coupling ImGui panels to renderer ownership.
- Identify the smallest UI/read-only API slice needed for an asset browser or inspector.
- Do not edit files unless reassigned by the parent agent.

### Scene Package Graph Validation Agent

Mode: read-only.

Scope:

- `engine/ScenePackage.*`
- `application/RuntimeEngineWorldVerification.*`
- `application/RuntimeImportedAssetVerification.*`
- `tools/verify_pbr.ps1`
- scene package docs

Task:

- Audit remaining package graph risks after unknown/corrupted package probes.
- Focus on duplicate persistent ids, invalid parent links, self-parent/cycle attach, and cross-actor component parent references.
- Report recommended validation order and test gates.
- Do not edit files.

### RendererSubsystem Boundary Agent

Mode: read-only.

Scope:

- `engine/RendererSubsystem.*`
- `engine/Engine.*`
- `application/RuntimeApplicationShell.*`
- `application/RuntimeFrameRunner.*`
- `application/RuntimeFramePipeline.*`
- `application/RuntimeVerificationReport.*`
- `application/RuntimePBRRendererStatsVerification.*`

Task:

- Audit the smallest safe bridge between Engine-owned `RendererSubsystem` and the existing application renderer frame path.
- Identify ownership risks, null pointer risks, frame ordering risks, and the minimum verification signal that proves the subsystem participates in runtime frames.
- Do not edit files.

### Verification Contract Agent

Mode: read-only.

Scope:

- `tools/verify_pbr.ps1`
- `application/RuntimeVerificationReport.*`
- `application/RuntimePBRPreparedSceneStatsVerification.*`
- `application/RuntimePBRRendererStatsVerification.*`
- `worked.md`
- `work.md`
- `docs/engine_phase1_execution_plan.md`
- `docs/engine_project_book.md`
- `docs/engine_interface_design.md`

Task:

- Audit how the new RendererSubsystem frame bridge should be exposed in verification output.
- Identify script assertions that are strong enough to catch a disconnected subsystem but stable enough for all existing verification modes.
- Do not edit files.

### RendererSubsystem Next Slice Agent

Mode: read-only.

Scope:

- `engine/RendererSubsystem.*`
- `application/RuntimeFrameRunner.*`
- `application/RuntimeFramePipeline.*`
- `application/RuntimeApplicationShell.*`
- `docs/engine_phase1_execution_plan.md`
- `docs/engine_project_book.md`
- `docs/engine_interface_design.md`

Task:

- Audit the smallest next slice after runtime pipeline pass stats.
- Compare two options: adding a bridge adapter boundary versus adding pass profile/profile-key stats first.
- Identify ownership, dependency, and verification risks.
- Do not edit files.

### Verification Continuity Agent

Mode: read-only.

Scope:

- `tools/verify_pbr.ps1`
- `application/RuntimeVerificationReport.*`
- `application/RuntimePBRRendererStatsVerification.*`
- `tools/editor/DebugControllerPanel.*`
- `work.md`
- `worked.md`

Task:

- Audit whether the current `framePasses=planned/executed/skipped` contract is stable across all verification modes.
- Recommend one minimal next assertion that proves the next renderer boundary slice is connected without overfitting to a specific scene.
- Do not edit files.

### Engine Diagnostics Health Counter Agent

Mode: read-only.

Scope:

- `engine/Engine.*`
- `engine/World.*`
- `engine/AssetSubsystem.*`
- `engine/RendererSubsystem.*`
- `application/AppRuntimeContext.h`
- `application/RuntimeVerificationReport.*`
- `application/RuntimePBRRendererStatsVerification.*`
- `tools/editor/EngineDiagnosticsPanel.*`
- `tools/verify_pbr.ps1`
- `docs/engine_phase1_execution_plan.md`

Task:

- Audit the smallest subsystem health counter slice after Engine diagnostics unified context.
- Identify counters that are stable across all runtime verification modes and useful for engine architecture, not just UI display.
- Recommend one or two verification assertions that prove Engine diagnostics observes real subsystem state.
- Do not edit files.

### Renderer Frame Contract Boundary Agent

Mode: read-only.

Scope:

- `engine/RendererSubsystem.*`
- `application/RuntimeRendererFrameBridgeAdapter.*`
- `application/RuntimeFramePipeline.*`
- `application/RuntimeFrameRunner.*`
- `application/RuntimeVerificationReport.*`
- `application/RuntimePBRRendererStatsVerification.*`
- `tools/verify_pbr.ps1`
- `docs/engine_interface_design.md`

Task:

- Audit the current `RendererFrameIntent` / `RendererFrameResult` boundary and the compatibility aliases in `RendererBackend.h`.
- Identify the smallest next contract improvement that moves renderer ownership toward the Engine without adding new PBR features.
- Call out fields or abstractions that would couple Engine too tightly to the legacy application pipeline.
- Do not edit files.

### Renderer Backend Contract Agent

Mode: read-only.

Scope:

- `engine/RendererSubsystem.*`
- `application/RuntimeRendererFrameBridgeAdapter.*`
- `application/RuntimeApplicationShell.*`
- `application/RuntimeFrameRunner.*`
- `tools/editor/EngineDiagnosticsPanel.*`

Task:

- Audit the active renderer backend metadata contract slice.
- Confirm whether backend key and backend readiness are exposed at the correct Engine/Application boundary.
- Identify ownership, lifetime, or null-state risks introduced by the non-owning executor attachment.
- Do not edit files.

### Renderer Backend Verification Agent

Mode: read-only.

Scope:

- `application/RuntimeVerificationReport.*`
- `application/RuntimePBRRendererStatsVerification.*`
- `tools/verify_pbr.ps1`
- `work.md`
- `worked.md`
- `docs/engine_phase1_execution_plan.md`
- `docs/engine_project_book.md`
- `docs/engine_interface_design.md`

Task:

- Audit the verification and documentation surface for the renderer backend metadata contract.
- Recommend the smallest stable assertions that prove a backend is attached and ready without overfitting to one scene.
- Confirm that documentation wording stays focused on engine architecture and does not expand PBR scope.
- Do not edit files.

### Renderer Backend No-op Verification Agent

Mode: read-only.

Scope:

- `tools/verify_pbr.ps1`
- `application/RuntimePBRVerificationArgs.cpp`
- `application/RuntimeVerificationReport.cpp`
- `application/RuntimePBRRendererStatsVerification.cpp`
- renderer backend registry docs

Task:

- Audit the script and argument surface needed for a dedicated no-op/test renderer backend verification mode.
- Identify which assertions must stay generic and which assertions must be conditional for a backend that clears the framebuffer without invoking the legacy `RuntimeFramePipeline`.
- Do not edit files; report exact script/code locations and the smallest safe verification contract.

### Engine World Cleanup Verification Agent

Mode: read-only.

Scope:

- `engine/Engine.*`
- `engine/World.*`
- `application/AppRuntimeContext.h`
- `application/RuntimeApplicationShell.*`
- `application/RuntimeVerificationReport.*`
- `application/RuntimeEngineWorldVerification.*`
- `tools/verify_pbr.ps1`

Task:

- Audit the smallest stable verification contract proving that Engine-owned active World state is ended/reset during runtime cleanup.
- Identify which output fields should be asserted globally across all verification modes.
- Call out cleanup ordering risks between runtime context pointer clearing, renderer subsystem cleanup, and `Engine::shutdown()`.
- Do not edit files.

### Runtime Content Lifecycle Audit Agent

Mode: read-only.

Scope:

- `application/RuntimeContentLifecycle.*`
- `application/RuntimeApplicationShell.*`
- `application/RuntimeScenePreparer.*`
- `application/RuntimeEngineLifecycle.*`
- `application/RuntimeVerificationLifecycle.*`
- `tools/verify_pbr.ps1`

Task:

- Audit whether startup content composition has moved out of `RuntimeApplicationShell` without changing render behavior.
- Check for ownership/lifetime risks in the ordering of camera initialization, profile load, verification startup profile, scene preparation, renderer backend attachment, and prepared-scene reporting.
- Identify missing focused verification or stale direct dependencies that would keep the shell coupled to scene/profile/PBR details.
- Do not edit files.

### Prepared Scene Verification Extraction Agent

Mode: read-only.

Scope:

- `application/RuntimePBRPreparedSceneStatsVerification.*`
- `application/RuntimeEngineWorldVerification.*`
- `application/RuntimeVerificationLifecycle.*`
- `application/RuntimeVerificationReport.*`
- `engine/ScenePackage.*`
- `tools/verify_pbr.ps1`
- `work.md`
- `worked.md`
- `docs/engine_phase1_execution_plan.md`
- `docs/engine_project_book.md`
- `docs/engine_interface_design.md`

Task:

- Audit that prepared-scene responsibilities remain split between neutral Engine World verification and PBR prepared-scene stats verification.
- Identify which Engine World snapshot, scene package round-trip, and negative package probe reports can move to a neutral verification module without changing PBR renderer behavior.
- Recommend the smallest next extraction boundary, exact candidate file names, and stable verification gates.
- Do not edit files.

### Prepared Scene Stats Neutralization Agent

Mode: read-only.

Scope:

- `application/RuntimePBRPreparedSceneStatsVerification.cpp`
- `application/RuntimeEngineWorldVerification.cpp`
- `application/RuntimeVerificationLifecycle.cpp`
- `tools/verify_pbr.ps1`
- `work.md`
- `worked.md`
- `docs/engine_phase1_execution_plan.md`
- `docs/engine_project_book.md`
- `docs/engine_interface_design.md`

Task:

- Audit whether Engine World prepared-scene counters have moved out of the PBR verification stats surface cleanly.
- Confirm that `engineWorldProbeMeshes`, `engineWorldMinimalMeshes`, and `runtimeWorldActors` are read from the neutral Engine World stats line instead of the PBR scene stats line.
- Identify missing assertions, stale documentation wording, or coupling risks.
- Do not edit files.

### Runtime PBR Scene Probe Verification Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimePBRSceneProbeVerification.*`
- `application/RuntimePBRPreparedSceneStatsVerification.*`
- `application/RuntimePBRVerificationConfig.h`
- `application/RuntimeVerificationLifecycle.*`
- `tools/verify_pbr.ps1`
- `work.md`
- `worked.md`
- `docs/engine_phase1_execution_plan.md`
- `docs/engine_project_book.md`
- `docs/engine_interface_design.md`

Task:

- Audit whether PBR verification scene probe construction remains separated from PBR prepared scene stats.
- Confirm transparent/emissive/material IBL/alpha mask/texture set/showcase probes are covered by focused verification modes.
- Report stale documentation or ownership drift without editing files unless assigned a disjoint write scope.

### Runtime PBR Prepared Scene Stats Verification Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimePBRPreparedSceneStatsVerification.*`
- `application/RuntimePBRVerificationConfig.h`
- `application/RuntimeVerificationLifecycle.*`
- `tools/verify_pbr.ps1`
- `work.md`
- `worked.md`
- `docs/engine_phase1_execution_plan.md`
- `docs/engine_project_book.md`
- `docs/engine_interface_design.md`

Task:

- Audit whether `PBR verification scene stats` remains a stable output contract after moving implementation out of the old PBR verification class.
- Confirm the old `RuntimePBRVerification` class, `.cpp`, and `.h` are gone while `RuntimePBRVerificationConfig` remains available through `RuntimePBRVerificationConfig.h` to `RuntimeVerificationConfig`.
- Report stale documentation or ownership drift without editing files unless assigned a disjoint write scope.

### Runtime PBR Pass Profile Verification Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimePBRPassProfileVerification.*`
- `application/RuntimePBRProfileVerification.*`
- `application/RuntimeVerificationLifecycle.*`
- `application/RuntimePBRVerificationConfig.h`
- `tools/verify_pbr.ps1`
- `work.md`
- `worked.md`
- `docs/engine_phase1_execution_plan.md`
- `docs/engine_project_book.md`
- `docs/engine_interface_design.md`

Task:

- Audit whether renderer pass profile writes are cleanly separated from startup profile, preview, light/camera rig, and profile line responsibilities.
- Confirm the prepared-scene lifecycle refresh now calls `RuntimePBRPassProfileVerification` directly instead of routing through `RuntimePBRProfileVerification`.
- Report stale documentation, ownership drift, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime PBR Profile Line Verification Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimePBRProfileLineVerification.*`
- `application/RuntimePBRProfileVerification.*`
- `application/RuntimePBRVerificationConfig.h`
- `tools/verify_pbr.ps1`
- `work.md`
- `worked.md`
- `docs/engine_phase1_execution_plan.md`
- `docs/engine_project_book.md`
- `docs/engine_interface_design.md`

Task:

- Audit whether `PBR verification profile applied` reporting is separated from startup profile writes without changing output text.
- Confirm logger/stdout dependencies are isolated to `RuntimePBRProfileLineVerification`.
- Report stale documentation, ownership drift, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime PBR Preview Profile Verification Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimePBRPreviewProfileVerification.*`
- `application/RuntimePBRProfileVerification.*`
- `application/RuntimePBRVerificationConfig.h`
- `tools/verify_pbr.ps1`
- `work.md`
- `worked.md`
- `docs/engine_phase1_execution_plan.md`
- `docs/engine_project_book.md`
- `docs/engine_interface_design.md`

Task:

- Audit whether PBR preview grid policy is separated from startup profile composition without changing default grid, showcase, or minimal-scene behavior.
- Confirm `RuntimePBRProfileVerification` no longer writes `context.pbrPreviewProfile` directly.
- Report stale documentation, ownership drift, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime PBR Light Camera Rig Verification Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimePBRProfileVerification.*`
- `application/RuntimePBRLightCameraRigVerification.*`
- `application/RuntimePBRVerificationConfig.h`
- `tools/verify_pbr.ps1`
- `work.md`
- `worked.md`
- `docs/engine_phase1_execution_plan.md`
- `docs/engine_project_book.md`
- `docs/engine_interface_design.md`

Task:

- Audit whether PBR verification light/camera rig policy is separated from startup profile composition without changing default, minimal-scene, tiled-light, showcase, or pressure-light behavior.
- Confirm `RuntimePBRProfileVerification` no longer writes `context.pbrLightRigProfile` or `context.pbrCameraRigProfile` directly after extraction.
- Report stale documentation, ownership drift, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime PBR Startup Profile Verification Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimePBRProfileVerification.*`
- `application/RuntimePBRStartupProfileVerification.*`
- `application/RuntimePBRPassProfileVerification.*`
- `application/RuntimePBRPreviewProfileVerification.*`
- `application/RuntimePBRLightCameraRigVerification.*`
- `tools/verify_pbr.ps1`
- `work.md`
- `worked.md`
- `docs/engine_phase1_execution_plan.md`
- `docs/engine_project_book.md`
- `docs/engine_interface_design.md`

Task:

- Audit whether environment, post-process, and runtime frame pipeline startup defaults are separated from PBR verification startup profile orchestration without changing render behavior.
- Confirm `RuntimePBRProfileVerification` no longer writes `context.environmentProfile`, `context.postProcessSettings`, or `context.framePipelineProfile` directly after extraction.
- Report stale documentation, ownership drift, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime Content Lifecycle Config Boundary Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimeContentLifecycle.*`
- `application/RuntimeContentLifecycleConfig.h`
- `application/RuntimeCameraConfig.h`
- `application/RuntimeScenePrepareConfig.h`
- `application/RuntimeContentConfigPolicy.*`
- `application/RuntimeCameraLifecycle.*`
- `application/RuntimeScenePreparer.*`
- `tools/verify_pbr.ps1`
- project docs

Task:

- Audit whether content startup config DTOs are separated from implementation lifecycle headers without changing startup behavior.
- Confirm public content lifecycle headers do not pull in camera lifecycle, scene preparer, engine lifecycle, verification lifecycle, or legacy experiment implementation dependencies.
- Report stale documentation, ownership drift, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime Content Renderer Backend Lifecycle Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimeContentLifecycle.*`
- `application/RuntimeContentRendererBackendLifecycle.*`
- `application/RuntimeEngineLifecycle.*`
- `application/AppRuntimeContext.h`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- `tools/verify_pbr.ps1`
- project docs

Task:

- Audit whether the post-scene-preparation renderer fail-fast gate and Engine-owned renderer backend attachment are separated from generic content composition without changing startup behavior.
- Confirm `RuntimeContentLifecycle.cpp` no longer directly calls `RuntimeEngineLifecycle::attachRendererBackend(...)` or logs the scene-prepare renderer failure after extraction.
- Report stale documentation, ownership drift, missing project registration, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime Content Verification Lifecycle Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimeContentLifecycle.*`
- `application/RuntimeContentVerificationLifecycle.*`
- `application/RuntimeProfileLoader.*`
- `application/RuntimeVerificationLifecycle.*`
- `application/RuntimeVerificationConfig.h`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- `tools/verify_pbr.ps1`
- project docs

Task:

- Audit whether runtime profile loading, verification startup profile application, and prepared-scene reporting are separated from generic content composition without changing startup behavior.
- Confirm `RuntimeContentLifecycle.cpp` no longer directly calls `RuntimeProfileLoader::loadAll(...)`, `RuntimeVerificationLifecycle::applyStartupProfile(...)`, or `RuntimeVerificationLifecycle::reportPreparedScene(...)`.
- Report stale documentation, ownership drift, missing project registration, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime Content Scene Lifecycle Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimeContentLifecycle.*`
- `application/RuntimeContentSceneLifecycle.*`
- `application/RuntimeScenePreparer.*`
- `application/RuntimeScenePrepareConfig.h`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- `tools/verify_pbr.ps1`
- project docs

Task:

- Audit whether scene preparation is separated from generic content composition without changing scene setup pipeline, legacy experiment, or Engine World prepared scene behavior.
- Confirm `RuntimeContentLifecycle.cpp` no longer directly calls `RuntimeScenePreparer::prepare(...)` or includes `RuntimeScenePreparer.h`.
- Report stale documentation, ownership drift, missing project registration, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime Content Camera Lifecycle Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimeContentLifecycle.*`
- `application/RuntimeContentCameraLifecycle.*`
- `application/RuntimeCameraLifecycle.*`
- `application/RuntimeCameraConfig.h`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- `tools/verify_pbr.ps1`
- project docs

Task:

- Audit whether startup camera initialization is separated from generic content composition without changing camera creation, camera control binding, or cleanup behavior.
- Confirm `RuntimeContentLifecycle.cpp` no longer directly calls `RuntimeCameraLifecycle::initializeDefaultCamera(...)` or includes `RuntimeCameraLifecycle.h`.
- Report stale documentation, ownership drift, missing project registration, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime Legacy Experiment Lifecycle Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimeScenePreparer.*`
- `application/RuntimeLegacyExperimentLifecycle.*`
- `application/RuntimeFrameRunner.*`
- `application/RuntimeScenePrepareConfig.h`
- `tools/legacyExperiments/LegacyExperimentRunner.h`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- `tools/verify_pbr.ps1`
- project docs

Task:

- Audit whether legacy experiment context construction, startup enable hooks, and per-frame update are separated from scene preparation without changing default disabled behavior.
- Confirm `RuntimeFrameRunner.cpp` no longer includes `RuntimeScenePreparer.h` or calls `RuntimeScenePreparer::updateLegacyExperiments(...)` after extraction.
- Report stale documentation, ownership drift, missing project registration, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Scene Setup Pipeline Config Header Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `tools/sceneSetup/SceneSetupPipeline.h`
- `tools/sceneSetup/SceneSetupPipelineConfig.h`
- `application/RuntimeScenePrepareConfig.h`
- `application/RuntimeScenePreparer.cpp`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- `tools/verify_pbr.ps1`
- project docs

Task:

- Audit whether scene setup pipeline config is separated from the full scene setup pipeline API without changing scene setup behavior.
- Confirm `RuntimeScenePrepareConfig.h` no longer includes full `SceneSetupPipeline.h`.
- Report stale documentation, ownership drift, missing project registration, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime Scene Setup Report Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimeScenePreparer.*`
- `application/RuntimeSceneSetupReport.*`
- `tools/sceneSetup/SceneSetupPipeline.*`
- `tools/sceneSetup/WorldDrivenSceneSetup.*`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- `tools/verify_pbr.ps1`
- project docs

Task:

- Audit whether scene setup result reporting and renderer prepared logging are separated from scene preparation without changing output content.
- Confirm `RuntimeScenePreparer.cpp` no longer directly includes logger/stdout or scene setup stats formatter calls.
- Report stale documentation, ownership drift, missing project registration, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime Scene Setup Context Factory Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimeScenePreparer.*`
- `application/RuntimeSceneSetupContextFactory.*`
- `application/AppRuntimeContext.h`
- `tools/sceneSetup/SceneSetup.h`
- `tools/sceneSetup/SceneSetupPipeline.*`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- `tools/verify_pbr.ps1`
- project docs

Task:

- Audit whether `AppRuntimeContext` to `GL_SCENE::SetupContext` assembly is separated from scene preparation without changing field mapping.
- Confirm `RuntimeScenePreparer` no longer exposes `makeSceneSetupContext(...)` and no longer directly includes `AppRuntimeContext.h` or `Engine.h`.
- Report stale documentation, ownership drift, missing project registration, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime Scene Setup Pipeline Lifecycle Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimeScenePreparer.*`
- `application/RuntimeSceneSetupPipelineLifecycle.*`
- `application/RuntimeSceneSetupContextFactory.*`
- `application/RuntimeSceneSetupReport.*`
- `tools/sceneSetup/SceneSetupPipeline.*`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- `tools/verify_pbr.ps1`
- project docs

Task:

- Audit whether scene setup pipeline execution is separated from scene preparation orchestration without changing setup behavior.
- Confirm `RuntimeScenePreparer` no longer directly includes full `SceneSetupPipeline.h` or calls `GL_SCENE::prepareScene(...)`.
- Report stale documentation, ownership drift, missing project registration, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime Scene Preparer Removal Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimeContentSceneLifecycle.*`
- deleted `application/RuntimeScenePreparer.*` references
- `application/RuntimeSceneSetupPipelineLifecycle.*`
- `application/RuntimeLegacyExperimentLifecycle.*`
- `application/RuntimeSceneSetupReport.*`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- `tools/verify_pbr.ps1`
- project docs

Task:

- Audit whether the now-empty scene preparer wrapper has been removed without changing scene preparation order.
- Confirm live source/project registration no longer references `RuntimeScenePreparer.h/.cpp`.
- Report stale documentation, ownership drift, missing project registration, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime Application Content Startup Lifecycle Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimeApplicationStartupLifecycle.*`
- `application/RuntimeApplicationContentStartupLifecycle.*`
- `application/RuntimeContentConfigPolicy.*`
- `application/RuntimeContentLifecycle.*`
- `application/RuntimeWindowLifecycle.*`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- `tools/verify_pbr.ps1`
- project docs

Task:

- Audit whether application startup no longer directly owns the content config policy and content lifecycle prepare bridge.
- Confirm startup order remains Engine, Window, Graphics, Content, Editor, Frame reset.
- Report stale documentation, ownership drift, missing project registration, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime Application Editor Startup Lifecycle Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimeApplicationStartupLifecycle.*`
- `application/RuntimeApplicationEditorStartupLifecycle.*`
- `application/RuntimeApplicationConfigPolicy.*`
- `application/RuntimeEditorLifecycle.*`
- `application/RuntimeWindowLifecycle.*`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- `tools/verify_pbr.ps1`
- project docs

Task:

- Audit whether application startup no longer directly owns editor lifecycle config creation or editor initialization.
- Confirm startup order remains Engine, Window, Graphics, Content, Editor, Frame reset.
- Report stale documentation, ownership drift, missing project registration, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime Application Frame Startup Lifecycle Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimeApplicationStartupLifecycle.*`
- `application/RuntimeApplicationFrameStartupLifecycle.*`
- `application/RuntimeFrameLifecycle.*`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- `tools/verify_pbr.ps1`
- project docs

Task:

- Audit whether application startup no longer directly owns frame lifecycle reset.
- Confirm startup order remains Engine, Window, Graphics, Content, Editor, Frame reset.
- Report stale documentation, ownership drift, missing project registration, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime Application Engine Startup Lifecycle Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimeApplicationStartupLifecycle.*`
- `application/RuntimeApplicationEngineStartupLifecycle.*`
- `application/RuntimeApplicationConfigPolicy.*`
- `application/RuntimeEngineLifecycle.*`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- `tools/verify_pbr.ps1`
- project docs

Task:

- Audit whether application startup no longer directly owns Engine desc mapping or Engine lifecycle initialization.
- Confirm startup order remains Engine, Window, Graphics, Content, Editor, Frame reset.
- Report stale documentation, ownership drift, missing project registration, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime Application Window Startup Lifecycle Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimeApplicationStartupLifecycle.*`
- `application/RuntimeApplicationWindowStartupLifecycle.*`
- `application/RuntimeWindowLifecycle.*`
- `application/RuntimeGraphicsLifecycle.*`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- `tools/verify_pbr.ps1`
- project docs

Task:

- Audit whether application startup no longer directly owns window setup prompt, window initialization, or window snapshot capture.
- Confirm startup order remains Engine, Window, Graphics, Content, Editor, Frame reset.
- Report stale documentation, ownership drift, missing project registration, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime Application Graphics Startup Lifecycle Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimeApplicationStartupLifecycle.*`
- `application/RuntimeApplicationGraphicsStartupLifecycle.*`
- `application/RuntimeApplicationConfigPolicy.*`
- `application/RuntimeGraphicsLifecycle.*`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- `tools/verify_pbr.ps1`
- project docs

Task:

- Audit whether application startup no longer directly owns graphics config mapping or graphics lifecycle initialization.
- Confirm startup order remains Engine, Window, Graphics, Window snapshot, Content, Editor, Frame reset.
- Report stale documentation, ownership drift, missing project registration, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime Application Frame Editor Callback Bridge Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimeApplicationFrameLifecycle.*`
- `application/RuntimeApplicationFrameEditorCallbackBridge.*`
- `application/RuntimeApplicationConfigPolicy.*`
- `application/RuntimeEditorLifecycle.*`
- `application/RuntimeFrameLifecycle.*`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- `tools/verify_pbr.ps1`
- project docs

Task:

- Audit whether application frame lifecycle no longer directly owns editor callback config mapping or editor frame callback creation.
- Confirm frame loop behavior, GUI callback gating, and editor callback config capture semantics remain unchanged.
- Report stale documentation, ownership drift, missing project registration, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime Application Frame Continue/Run Bridge Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimeApplicationFrameLifecycle.*`
- `application/RuntimeApplicationFrameContinueBridge.*`
- `application/RuntimeApplicationFrameRunBridge.*`
- `application/RuntimeApplicationFrameEditorCallbackBridge.*`
- `application/RuntimeApplicationConfigPolicy.*`
- `application/RuntimeFrameLifecycle.*`
- `application/RuntimeWindowLifecycle.*`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- `tools/verify_pbr.ps1`
- project docs

Task:

- Audit whether application frame lifecycle no longer directly owns frame continue config mapping, window snapshot capture, frame execution parameter expansion, or direct `RuntimeFrameLifecycle` calls.
- Confirm frame loop behavior, GUI callback gating, verification capture, framebuffer size propagation, and renderer backend frame bridge behavior remain unchanged.
- Report stale documentation, ownership drift, missing project registration, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime Application Shutdown Cleanup/Destroy Bridge Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimeApplicationShutdownLifecycle.*`
- `application/RuntimeApplicationShutdownCleanupBridge.*`
- `application/RuntimeApplicationShutdownDestroyBridge.*`
- `application/RuntimeEngineLifecycle.*`
- `application/RuntimeVerificationLifecycle.*`
- `application/RuntimeCameraLifecycle.*`
- `application/RuntimeWindowLifecycle.*`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- `tools/verify_pbr.ps1`
- project docs

Task:

- Audit whether application shutdown lifecycle no longer directly owns renderer cleanup reporting, camera cleanup, runtime context detach, Engine shutdown, Engine cleanup reporting, or window destroy calls.
- Confirm cleanup order remains begin cleanup, renderer cleanup report, camera cleanup, runtime context detach, Engine shutdown, Engine cleanup report; confirm destroy still delegates to window destroy.
- Report stale documentation, ownership drift, missing project registration, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime Application Shutdown Verification Bridge Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimeApplicationShutdownCleanupBridge.*`
- `application/RuntimeApplicationShutdownVerificationBridge.*`
- `application/RuntimeEngineLifecycle.*`
- `application/RuntimeVerificationLifecycle.*`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- `tools/verify_pbr.ps1`
- project docs

Task:

- Audit whether shutdown cleanup verification report delegation is isolated behind `RuntimeApplicationShutdownVerificationBridge`.
- Confirm `RuntimeApplicationShutdownCleanupBridge.cpp` no longer directly includes or calls `RuntimeVerificationLifecycle`.
- Confirm cleanup order remains begin cleanup, renderer cleanup report, camera cleanup, runtime context detach, Engine shutdown, Engine cleanup report.
- Report stale documentation, ownership drift, missing project registration, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime Application Shutdown Engine Bridge Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimeApplicationShutdownCleanupBridge.*`
- `application/RuntimeApplicationShutdownEngineBridge.*`
- `application/RuntimeApplicationShutdownVerificationBridge.*`
- `application/RuntimeEngineLifecycle.*`
- `application/RuntimeCameraLifecycle.*`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- `tools/verify_pbr.ps1`
- project docs

Task:

- Audit whether shutdown begin cleanup, camera cleanup, runtime context detach, and Engine shutdown delegation are isolated behind `RuntimeApplicationShutdownEngineBridge`.
- Confirm `RuntimeApplicationShutdownCleanupBridge.cpp` no longer directly includes or calls `RuntimeCameraLifecycle` or `RuntimeEngineLifecycle`.
- Confirm `RuntimeApplicationShutdownVerificationBridge.*` consumes the canonical `RuntimeEngineLifecycleCleanupRefs`; the earlier application-level `RuntimeApplicationShutdownCleanupRefs` boundary has been superseded.
- Confirm cleanup order remains begin cleanup, renderer cleanup report, runtime context cleanup, Engine shutdown, Engine cleanup report.
- Report stale documentation, ownership drift, missing project registration, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime Engine Lifecycle Types Header Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimeEngineLifecycle.h`
- `application/RuntimeEngineLifecycleTypes.h`
- `application/RuntimeEngineLifecycle.cpp`
- `application/RuntimeApplicationState.h`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- `tools/verify_pbr.ps1`
- project docs

Task:

- Audit whether `RuntimeEngineLifecycleState` and `RuntimeEngineLifecycleCleanupRefs` are isolated behind `RuntimeEngineLifecycleTypes.h`.
- Confirm `RuntimeEngineLifecycle.h` no longer public-includes `AppRuntimeContext.h`, `Engine.h`, `AssetSubsystem.h`, or `RendererSubsystem.h`.
- Confirm `RuntimeEngineLifecycle.cpp` owns the implementation-only includes needed for `AppRuntimeContext`, `Engine`, `AssetSubsystem`, and `RendererSubsystem`.
- Confirm focused and full verification still cover startup, frame, shutdown, and cleanup reporting paths.
- Report stale documentation, ownership drift, missing project registration, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime Application State Forward Boundary Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimeApplicationShell.h`
- `application/RuntimeApplicationShell.cpp`
- `application/RuntimeApplicationState.h`
- `application/RuntimeApplicationCallbackBinder.*`
- `application/RuntimeApplication*Lifecycle.h`
- `application/RuntimeApplication*Bridge.h`
- `tools/verify_pbr.ps1`
- project docs

Task:

- Audit whether `RuntimeApplicationShell.h` hides the complete `RuntimeApplicationState` type behind `std::unique_ptr` and no longer public-includes `RuntimeApplicationState.h`.
- Confirm application startup/frame/shutdown lifecycle and bridge headers that only take state/config by reference forward-declare those types instead of including full application state/config headers.
- Confirm implementation files explicitly include the complete state/config headers needed for field access.
- Confirm focused and full verification still cover startup, frame, shutdown, callback binding, and cleanup reporting paths.
- Report stale documentation, ownership drift, missing project registration, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime Profile State Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/AppRuntimeContext.h`
- `application/RuntimeProfileState.h`
- `application/RuntimeProfileLoader.*`
- `application/RuntimeEditorPanelCoordinator.cpp`
- `application/RuntimeFramePasses.cpp`
- `application/RuntimeFramePassRegistry.cpp`
- `application/RuntimeFramePipeline.cpp`
- `application/RuntimeRendererFrameBridgeAdapter.cpp`
- `application/RuntimeSceneSetupContextFactory.cpp`
- `application/RuntimePBR*Verification.cpp`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- `tools/verify_pbr.ps1`
- project docs

Task:

- Audit whether runtime profile/path data is isolated behind `RuntimeProfileState.h` and exposed through `AppRuntimeContext::profiles`.
- Confirm `AppRuntimeContext.h` no longer directly includes frame pipeline profile, environment profile, post-process settings, renderer frame pass profile, or PBR scene setup profile headers except through `RuntimeProfileState.h`.
- Confirm application code no longer uses top-level `context.framePipelineProfile`, `context.postProcessSettings`, `context.environmentProfile`, `context.pbrPreviewProfile`, `context.pbrLightRigProfile`, `context.pbrCameraRigProfile`, or related profile path fields.
- Confirm profile load order, frame pass policy, PBR verification policy, scene setup mapping, and renderer backend readiness behavior are preserved by focused and full verification.
- Report stale documentation, ownership drift, missing project registration, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime Render Resource State Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/AppRuntimeContext.h`
- `application/RuntimeRenderResourceState.h`
- `application/RuntimeFramePasses.cpp`
- `application/RuntimeFrameRunner.cpp`
- `application/RuntimeRendererFrameBridgeAdapter.cpp`
- `application/RuntimeSceneSetupContextFactory.cpp`
- `application/RuntimeWindowLifecycle.cpp`
- `application/RuntimeEngineLifecycle.cpp`
- `application/RuntimeEditorPanelCoordinator.cpp`
- `application/RuntimeLegacyExperimentLifecycle.cpp`
- `application/RuntimePBR*Verification.cpp`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- project docs

Task:

- Audit whether renderer, scene, mesh/material, frame target, bloom, post-process pass, and clear color state is isolated behind `RuntimeRenderResourceState.h` and exposed through `AppRuntimeContext::renderResources`.
- Confirm application code no longer uses those render resource fields as top-level `AppRuntimeContext` members; `RuntimeViewport` resize DTO field names are not AppRuntimeContext fields and should not be treated as stale.
- Confirm camera/light fields remain intentionally outside this slice for the next `AppRuntimeContext camera-light boundary`.
- Confirm focused and full verification still cover runtime frame passes, resize callback wiring, renderer backend readiness, scene setup mapping, and verification report paths.
- Report stale documentation, ownership drift, missing project registration, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime Camera Light State Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/AppRuntimeContext.h`
- `application/RuntimeCameraLightState.h`
- `application/RuntimeCameraLifecycle.cpp`
- `application/RuntimeFramePasses.cpp`
- `application/RuntimeFrameRunner.cpp`
- `application/RuntimeRendererFrameBridgeAdapter.cpp`
- `application/RuntimeSceneSetupContextFactory.cpp`
- `application/RuntimeWindowLifecycle.cpp`
- `application/RuntimeEditorPanelCoordinator.cpp`
- `application/RuntimeLegacyExperimentLifecycle.cpp`
- `application/RuntimeProfileLoader.cpp`
- `application/RuntimePBRLightCameraRigVerification.cpp`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- project docs

Task:

- Audit whether camera, camera control, ambient light, directional light, spot light, and point light state is isolated behind `RuntimeCameraLightState.h` and exposed through `AppRuntimeContext::cameraLights`.
- Confirm application code no longer uses those fields as top-level `AppRuntimeContext` members; `RuntimeInputContext` and `RuntimeResizeContext` DTO field names are not AppRuntimeContext fields and should not be treated as stale.
- Confirm camera initialization/cleanup order, resize camera aspect sync, renderer scene-color pass arguments, scene setup light mapping, editor light/camera mapping, and PBR camera rig application are preserved by focused and full verification.
- Report stale documentation, ownership drift, missing project registration, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime Engine Attachment State Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/AppRuntimeContext.h`
- `application/RuntimeEngineAttachmentState.h`
- `application/RuntimeEngineLifecycle.cpp`
- `application/RuntimeEditorPanelCoordinator.cpp`
- `application/RuntimeEngineWorldVerification.cpp`
- `application/RuntimeImportedAssetVerification.cpp`
- `application/RuntimeSceneSetupContextFactory.cpp`
- `application/RuntimeVerificationReport.cpp`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- project docs

Task:

- Audit whether Engine, World, AssetSubsystem, RendererSubsystem, and engine-world editable state is isolated behind `RuntimeEngineAttachmentState.h` and exposed through `AppRuntimeContext::engineAttachments`.
- Confirm application code no longer uses those fields as top-level `AppRuntimeContext` members.
- Confirm Engine attach/detach, World editable gates, AssetSubsystem registry access, RendererSubsystem report checks, scene setup mapping, imported asset verification, and cleanup report output are preserved by focused and full verification.
- Report stale documentation, ownership drift, missing project registration, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime Renderer Backend Attachment Lifecycle Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimeRendererBackendAttachmentLifecycle.h`
- `application/RuntimeRendererBackendAttachmentLifecycle.cpp`
- `application/RuntimeEngineLifecycle.cpp`
- `application/RuntimeRendererBackendCatalog.h`
- `application/RuntimeRendererBackendCatalog.cpp`
- `application/RuntimeRendererBackendFactory.h`
- `application/RuntimeRendererBackendFactory.cpp`
- `engine/RendererSubsystem.h`
- `engine/RendererSubsystem.cpp`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- project docs

Task:

- Audit whether renderer backend selection, factory creation, attachment desc creation, and `RendererSubsystem::setRendererBackend(...)` are isolated behind `RuntimeRendererBackendAttachmentLifecycle`.
- Confirm `RuntimeEngineLifecycle.cpp` no longer directly depends on `RuntimeRendererBackendCatalog` or `RuntimeRendererBackendFactory`.
- Confirm runtime backend and no-op backend paths still prove Engine-owned backend attachment, readiness, registry metadata, frame result, and cleanup detach.
- Report stale documentation, ownership drift, missing project registration, or missing verification coverage without editing files unless assigned a disjoint write scope.

### RendererSubsystem Frame Bridge Stats Header Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `engine/RendererSubsystem.h`
- `engine/RendererSubsystem.cpp`
- `engine/RendererSubsystemFrameBridgeStats.h`
- `engine/RendererBackend.h`
- `application/RuntimeVerificationReport.*`
- `tools/editor/EngineDiagnosticsPanel.*`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- project docs

Task:

- Audit whether `RendererSubsystemFrameBridgeStats` is isolated behind `RendererSubsystemFrameBridgeStats.h` while preserving the existing `RendererSubsystem::getFrameBridgeStats()` contract.
- Confirm `RendererSubsystem.h` no longer owns the stats field list or direct `<string>` dependency.
- Confirm verification output, diagnostics UI, VS header registration, and filter placement remain compatible.
- Report stale documentation, include drift, naming risk, or missing verification coverage without editing files unless assigned a disjoint write scope.

### RendererSubsystem Frame Bridge State Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `engine/RendererSubsystem.h`
- `engine/RendererSubsystem.cpp`
- `engine/RendererSubsystemFrameBridgeState.h`
- `engine/RendererSubsystemFrameBridgeState.cpp`
- `engine/RendererSubsystemFrameBridgeStats.h`
- `engine/RendererBackend.h`
- `application/RuntimeVerificationReport.*`
- `tools/editor/EngineDiagnosticsPanel.*`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- project docs

Task:

- Audit whether frame bridge stats mutation policy is isolated behind `RendererSubsystemFrameBridgeState`.
- Confirm `RendererSubsystem` no longer directly mutates `RendererSubsystemFrameBridgeStats` fields and still owns lifecycle/backend dispatch.
- Confirm verification output, diagnostics UI, focused verification, full verification, VS source/header registration, and filter placement remain compatible.
- Report stale documentation, include drift, naming risk, or missing verification coverage without editing files unless assigned a disjoint write scope.

### RendererSubsystem Backend Slot Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `engine/RendererSubsystem.h`
- `engine/RendererSubsystem.cpp`
- `engine/RendererSubsystemBackendSlot.h`
- `engine/RendererSubsystemBackendSlot.cpp`
- `engine/RendererSubsystemFrameBridgeState.*`
- `engine/RendererBackend.h`
- `application/RuntimeRendererBackendAttachmentLifecycle.*`
- `application/RuntimeVerificationReport.*`
- `tools/editor/EngineDiagnosticsPanel.*`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- project docs

Task:

- Audit whether renderer backend ownership, attachment metadata normalization, backend readiness checks, and attach/detach change detection are isolated behind `RendererSubsystemBackendSlot`.
- Confirm `RendererSubsystem` keeps lifecycle/frame dispatch responsibility while no longer directly owning `std::unique_ptr<RendererBackend>` or `RendererBackendAttachmentDesc`.
- Confirm runtime backend, no-op backend, cleanup detach, verification output, diagnostics UI, VS source/header registration, and filter placement remain compatible.
- Report stale documentation, include drift, ownership drift, or missing verification coverage without editing files unless assigned a disjoint write scope.

### RendererSubsystem Backend Slot Snapshot Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `engine/RendererSubsystem.h`
- `engine/RendererSubsystem.cpp`
- `engine/RendererSubsystemBackendSlot.h`
- `engine/RendererSubsystemBackendSlot.cpp`
- `engine/RendererSubsystemFrameBridgeState.h`
- `engine/RendererSubsystemFrameBridgeState.cpp`
- `engine/RendererBackend.h`
- `application/RuntimeVerificationReport.*`
- `tools/editor/EngineDiagnosticsPanel.*`
- project docs

Task:

- Audit whether `RendererSubsystemFrameBridgeState` consumes backend observation through `RendererSubsystemBackendSlotSnapshot` rather than probing raw `RendererBackend*`.
- Confirm backend key fallback, readiness state, attached/detached metadata, registry count, attach/detach counters, runtime backend mode, no-op backend mode, and cleanup stats remain compatible.
- Report stale documentation, naming drift, API leak, or missing verification coverage without editing files unless assigned a disjoint write scope.

### RendererSubsystem Frame Execution Bridge Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `engine/RendererSubsystem.h`
- `engine/RendererSubsystem.cpp`
- `engine/RendererSubsystemFrameExecutionBridge.h`
- `engine/RendererSubsystemFrameExecutionBridge.cpp`
- `engine/RendererSubsystemBackendSlot.*`
- `engine/RendererSubsystemFrameBridgeState.*`
- `engine/RendererBackend.h`
- `application/RuntimeVerificationReport.*`
- `tools/editor/EngineDiagnosticsPanel.*`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- project docs

Task:

- Audit whether backend frame execution and default frame result generation are isolated behind `RendererSubsystemFrameExecutionBridge`.
- Confirm `RendererSubsystem` still owns lifecycle, begin/end bridge calls, ready/not-ready counters, exception path, and public facade responsibilities.
- Confirm runtime backend, no-op backend, cleanup stats, verification output, diagnostics UI, VS source/header registration, and filter placement remain compatible.
- Report stale documentation, execution-order drift, API leak, or missing verification coverage without editing files unless assigned a disjoint write scope.

### RendererSubsystem Renderer Backend API Naming Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `engine/RendererSubsystem.*`
- `engine/RendererSubsystemFrameBridgeStats.h`
- `engine/RendererSubsystemFrameBridgeState.*`
- `application/RuntimeRendererBackendAttachmentLifecycle.*`
- `application/RuntimeFrameRunner.*`
- `application/RuntimeEngineLifecycle.*`
- `application/RuntimeVerificationReport.*`
- `tools/editor/EngineDiagnosticsPanel.*`
- `tools/verify_pbr.ps1`
- project docs

Task:

- Audit whether live application code uses renderer backend API names instead of old frame executor names.
- Confirm old `FrameExecutor` public API wrappers are removed and old verification fields are compatibility-only synchronized output.
- Confirm runtime backend, no-op backend, cleanup stats, verification output, diagnostics UI, and focused/full verification remain compatible.
- Report stale documentation, compatibility drift, API leak, or missing verification coverage without editing files unless assigned a disjoint write scope.

### RendererSubsystem Renderer Backend Public Compatibility API Removal Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `engine/RendererBackend.h`
- `engine/RendererSubsystem.*`
- `engine/RendererSubsystemFrameBridgeStats.h`
- `engine/RendererSubsystemFrameBridgeState.*`
- `application/RuntimeVerificationReport.*`
- `tools/verify_pbr.ps1`
- project docs

Task:

- Audit whether old `FrameExecutor` public API wrappers and renderer DTO aliases have been removed from live engine/application/tools code.
- Confirm old `frameExecutor*` names remain only as verification/log compatibility fields and stay synchronized with renderer-backend fields.
- Confirm runtime backend, no-op backend, cleanup stats, verification output, diagnostics UI, and focused/full verification remain compatible.
- Report stale documentation, compatibility drift, API leak, or missing verification coverage without editing files unless assigned a disjoint write scope.

### RendererSubsystem Frame Executor Stats Internal Compatibility Removal Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `engine/RendererSubsystemFrameBridgeStats.h`
- `engine/RendererSubsystemFrameBridgeState.*`
- `application/RuntimeVerificationReport.*`
- `tools/verify_pbr.ps1`
- project docs

Task:

- Audit whether old `frameExecutor*` fields have been removed from Engine internal stats.
- Confirm old `frameExecutorAttached`, `frameExecutorCalls`, and `rendererFrameExecutorAttached` remain only as verification/log compatibility output derived from renderer-backend stats.
- Confirm runtime backend, no-op backend, cleanup stats, verification output, and focused/full verification remain compatible.
- Report stale documentation, compatibility drift, API leak, or missing verification coverage without editing files unless assigned a disjoint write scope.

### RendererSubsystem Frame Executor Verification Output Removal Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimeVerificationReport.*`
- `tools/verify_pbr.ps1`
- project docs

Task:

- Audit whether old `frameExecutor*` fields have been removed from live verification/report output and parser assertions.
- Confirm renderer backend verification still checks attach, readiness, frame-call counts, backend metadata, no-op backend behavior, cleanup detach, and focused/full verification.
- Confirm old `FrameExecutor` naming remains only in historical documentation records, not live engine/application/tools contract.
- Report stale documentation, compatibility drift, API leak, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime Renderer Backend Verification Report Boundary Audit Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimeVerificationReport.*`
- potential `application/RuntimeRendererBackendVerificationReport.*`
- `tools/verify_pbr.ps1`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- project docs

Task:

- Audit whether extracting renderer backend verification report line formatting into a dedicated application module is a low-risk Engine runtime boundary slice.
- Confirm the extraction should not change runtime verification output fields, parser assertions, or renderer backend cleanup contract behavior.
- Confirm the slice stays inside application verification/reporting and does not expand PBR feature scope.
- Report project registration, stale documentation, compatibility drift, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime Verification Prepared Scene Lifecycle Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimeVerificationLifecycle.*`
- potential `application/RuntimeVerificationPreparedSceneLifecycle.*`
- `application/RuntimeEngineWorldVerification.*`
- `application/RuntimePBRSceneProbeVerification.*`
- `application/RuntimeImportedAssetVerification.*`
- `application/RuntimePBRPassProfileVerification.*`
- `application/RuntimePBRPreparedSceneStatsVerification.*`
- `text2.vcxproj`
- `text2.vcxproj.filters`
- project docs

Task:

- Audit whether prepared-scene verification orchestration can be isolated behind a dedicated application lifecycle module without changing output order or verification behavior.
- Confirm the parent-owned extraction should leave `RuntimeVerificationLifecycle` as a facade and remove direct includes for Engine World/PBR scene probe/import/pass-profile/prepared-scene stats modules.
- Confirm the slice stays inside application verification orchestration and does not expand PBR feature scope.
- Report project registration, stale documentation, output-order drift, include drift, or missing verification coverage without editing files unless assigned a disjoint write scope.

### Runtime Frame Runner Config DTO Header Boundary Audit Agent

Mode: read-only unless explicitly reassigned.

Scope:

- `application/RuntimeFrameRunner.*`
- potential `application/RuntimeFrameRunnerTypes.h`
- `application/RuntimeFrameCallbacks.h`
- `application/RuntimeFrameLifecycle.*`
- `application/RuntimeFrameLifecycleConfig.h`
- `application/RuntimeFrameLifecycleState.h`
- application frame bridge headers that include runner/callback/config DTOs
- `text2.vcxproj`
- `text2.vcxproj.filters`
- project docs

Task:

- Audit whether Bloom public header dependencies can be narrowed without changing bloom framebuffer allocation, blur passes, bright extraction, composite pass, renderer backend contract output, or PBR pass behavior.
- Confirm the parent-owned extraction should not alter scene setup, post-process frame execution, runtime frame plan ordering, verification capture timing, renderer backend contract output, or PBR pass behavior.
- Confirm the slice remains a renderer header boundary cleanup and does not expand renderer/PBR scope.
- Report include drift, API leak, project registration requirements, stale documentation, or missing verification coverage without editing files unless assigned a disjoint write scope.

## Current Active Agent Round

Round: 2026-06-01 Runtime Render Resource PostProcessPass Owner Boundary Cleanup.

Parent local work:

- Owns the active goal and keeps implementation moving without redefining the goal.
- Owns source edits for the current slice and any integration that follows from the sidecar audit.
- Must keep the existing `RuntimeFramePipeline` render path operational and must not expand PBR feature scope unless explicitly required by the engine architecture.
- Must keep `imgui.ini` treated as unrelated local state.
- Current local implementation target for this slice: remove `PostProcessPass.h` propagation from `RuntimeRenderResourceState.h` by hiding the post-process pass owner behind an implementation-managed pointer, while keeping frame pass behavior unchanged.
- Parent owns final integration, verification commands, `work.md`, `worked.md`, and user-facing summary.

Delegated sidecar work:

- Sidecar subagents in this round are read-only unless the parent explicitly assigns a disjoint write scope.
- No active subagent has write ownership in this round.
- No new sidecar subagent is started in this round; the Runtime render resource post-process pass owner cleanup is parent-owned and has no delegated write scope.
- No active sidecar remains open in this round.
- Parent-owned write scope for this round: `application/RuntimeRenderResourceState.h`, `application/RuntimeRenderResourceState.cpp`, `application/RuntimeFramePasses.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, docs, and logs.
- Parent local work is not blocked on the sidecar audit; implementation, project registration, verification, and documentation remain parent-owned.
- Sidecar findings must be reported in the shared communication format and are not accepted until the parent records accepted work in `worked.md`.
- This round restarts/continues the active goal under the existing objective; no new goal is created while the current goal remains active.
- No sidecar has write ownership in this round; source/project/documentation edits remain parent-owned.
- Previous round's Runtime Profile State Storage Path Boundary Cleanup was parent-owned and had no delegated write scope.
- Previous round's Assimp Instance Loader GLM Header Boundary Cleanup was parent-owned and had no delegated write scope.
- Previous round's Application Header Boundary Cleanup was parent-owned and had no delegated write scope.
- Previous round's Runtime Frame Clock Private State PIMPL Cleanup was parent-owned and had no delegated write scope.
- Previous round's Legacy Experiment Runner Private State PIMPL Cleanup was parent-owned and had no delegated write scope.
- Previous round's Legacy Experiment Runner Implementation Split was parent-owned and had no delegated write scope.
- Previous round's Material Types Header Extraction was parent-owned and had no delegated write scope.
- Previous round's Assimp Loader Public Header Boundary Cleanup was parent-owned and had no delegated write scope.
- Previous round's PBR Material Profile Header Extraction was parent-owned and had no delegated write scope.
- Previous round's PBR Experiment Profile Header Boundary Cleanup was parent-owned and had no delegated write scope.
- Previous round's PBR Light Rig Profile Header Boundary Cleanup was parent-owned and had no delegated write scope.
- Previous round's Engine Actor Component Header Boundary Cleanup was parent-owned and had no delegated write scope.
- Previous round's Runtime Application Config Backend Key Default Boundary Cleanup was parent-owned and had no delegated write scope.
- Previous round's Runtime Frame Lifecycle State owner cleanup was parent-owned and had no delegated write scope.
- Previous round's Runtime Graphics Lifecycle Types header extraction was parent-owned and had no delegated write scope.
- Previous round's Runtime GUI Host Types header extraction was parent-owned and had no delegated write scope.
- Previous round's Runtime Window Lifecycle header cleanup was parent-owned and had no delegated write scope.
- Previous round's Runtime Viewport header cleanup was parent-owned and had no delegated write scope.
- Previous round's Runtime Editor Lifecycle State owner cleanup was parent-owned and had no delegated write scope.
- Previous round's Editor Diagnostics Context header extraction was parent-owned and had no delegated write scope.
- Previous round's Editor Selection State header extraction was parent-owned and had no delegated write scope.
- Previous round's Runtime Editor Panel Coordinator header cleanup was parent-owned and had no delegated write scope.
- Previous round's Editor Panels Public header cleanup was parent-owned and had no delegated write scope.
- Previous round's SceneSetup Context header cleanup was parent-owned and had no delegated write scope.
- Previous round's Renderer Facade PImpl header cleanup was parent-owned and had no delegated write scope.
- Previous round's Renderer Infrastructure and Runtime Input header cleanup accepted read-only sidecar `Pauli`'s `RuntimeInputController.h` recommendation; all source edits were parent-owned.
- Previous round's PBR Deferred Lighting Grid header cleanup was parent-owned and had no delegated write scope.
- Previous round's PBR Shadow Atlas Render Pass header cleanup was parent-owned and had no delegated write scope.
- Previous round's Scene Render Pass header cleanup was parent-owned and had no delegated write scope.
- Previous round's PBR Draw Pass header cleanup was parent-owned and had no delegated write scope.
- Previous round's Draw Helper Debug Quad header cleanup was parent-owned and had no delegated write scope.
- Previous round's Shadow Render Pass header cleanup was parent-owned and had no delegated write scope.
- Previous round's PBR Alpha Shadow Binder header cleanup was parent-owned and had no delegated write scope.
- Previous round's Shadow Resource Binder header cleanup was parent-owned and had no delegated write scope.
- Previous round's PBR IBL Resource Binder header cleanup was parent-owned and had no delegated write scope.
- Previous round's PBR Surface Resource Binder header cleanup was parent-owned and had no delegated write scope.
- Previous round's PBR Shadow Resource Binder header cleanup was parent-owned and had no delegated write scope.
- Previous round's PBR Object Uniform Binder header cleanup was parent-owned and had no delegated write scope.
- Previous round's PBR Material Binder header cleanup was parent-owned and had no delegated write scope.
- Previous round's Material Binder header cleanup was parent-owned and had no delegated write scope.
- Previous round's Depth Prepass Binder header cleanup was parent-owned and had no delegated write scope.
- Previous round's Light Resource Binder header cleanup was parent-owned and had no delegated write scope.
- Previous round's IBL precompute pass header cleanup was parent-owned and had no delegated write scope.
- Previous round's Environment texture header cleanup was parent-owned and had no delegated write scope.
- Previous round's Bloom header framebuffer cleanup was parent-owned and had no delegated write scope.
- Previous round's PostProcess Pass header cleanup was parent-owned and had no delegated write scope.
- Previous round's Frame Render Targets framebuffer header cleanup was parent-owned and had no delegated write scope.
- Previous round's Engine Lifecycle Snapshot header cleanup was parent-owned and had no delegated write scope.
- Previous round's Runtime Frame Pass Registry key string_view cleanup was parent-owned and had no delegated write scope.
- Previous round's Runtime Frame Pipeline context header cleanup was parent-owned and had no delegated write scope.
- Previous round's Renderer Backend Contract frame DTO header cleanup was parent-owned and had no delegated write scope.
- Previous round's Engine ScenePackage Load Result World owner cleanup was parent-owned and had no delegated write scope.
- Previous round's Engine Legacy Scene Transform header cleanup was parent-owned and had no delegated write scope.
- Previous round's Engine Level Actor header cleanup was parent-owned and had no delegated write scope.
- Previous round's Runtime renderer backend catalog registry object API cleanup was parent-owned and had no delegated write scope.
- Previous round's Runtime renderer backend catalog registry header cleanup was parent-owned and had no delegated write scope.
- Previous round's RendererSubsystem backend slot header cleanup was parent-owned and had no delegated write scope.
- Previous round's Renderer Backend frame types header extraction was parent-owned and had no delegated write scope.
- Previous round's Engine AssetSubsystem registry header cleanup was parent-owned and had no delegated write scope.
- Previous round's Engine Actor root SceneComponent header cleanup was parent-owned and had no delegated write scope.
- Previous round's Engine World persistent level header cleanup was parent-owned and had no delegated write scope.
- Previous round's runtime application shell config header cleanup was parent-owned and had no delegated write scope.
- Previous round's runtime frame callback default-argument header cleanup was parent-owned and had no delegated write scope.
- Previous round's RendererSubsystem implementation-state header boundary cleanup was parent-owned after read-only `Einstein` audit and had no delegated write scope.
- Completed read-only sidecar subagent `Einstein` audited the RendererSubsystem implementation-state header boundary, changed no files, found no source blocker, confirmed incomplete-type `std::unique_ptr` handling and explicit complete-type call-site includes, and reported missing `work.md` / `worked.md` current-round records that the parent integrated.
- Previous round's Engine addSubsystem context helper cleanup was parent-owned and had no delegated write scope.
- Previous round's Engine public header context ownership boundary cleanup was parent-owned after read-only `Erdos` audit and had no delegated write scope.
- Completed read-only sidecar subagent `Erdos` audited `Engine.h` / `Engine.cpp`, changed no files, found no blocking issue, confirmed forward declarations and out-of-line `EngineContext` construction are sufficient, and reported only a non-blocking optional helper if a stricter no-public-header-dereference boundary is desired later.
- Previous round's runtime bootstrapper callbacks header extraction was parent-owned and had no delegated write scope.
- Previous round's callback binder startup bridge boundary cleanup was parent-owned after read-only `Singer` audit and had no delegated write scope.
- Previous round's callback binder frame bridge boundary cleanup was parent-owned and had no delegated write scope.
- Previous round's callback binder shutdown bridge boundary cleanup was parent-owned and had no delegated write scope.
- Previous round's shutdown lifecycle verification config boundary cleanup was parent-owned and had no delegated write scope.
- Previous round's shutdown verification config boundary cleanup was parent-owned and had no delegated write scope.
- Previous round's shutdown cleanup verification config boundary cleanup was parent-owned and had no delegated write scope.
- Previous round's engine desc header extraction was parent-owned and had no delegated write scope.
- Previous round's engine run mode header extraction was parent-owned and had no delegated write scope.
- Previous round's engine lifecycle snapshot type header extraction was parent-owned and had no delegated write scope.
- Previous round's runtime editor lifecycle callback DTO header boundary cleanup was parent-owned after read-only `Galileo` recommendation and had no delegated write scope.
- Completed read-only sidecar subagent `Harvey` audited verification/report include surface, changed no files, found no direct include deletion candidate, and recommended deferring to a larger `EngineLifecycleSnapshot` type-header extraction if a later round accepts the broader engine-header/project-registration risk.
- Completed read-only sidecar subagent `Galileo` audited application startup/frame/shutdown bridge include surface, changed no files, and recommended the lower-risk `RuntimeEditorLifecycle.h` callback DTO forward-declaration cleanup accepted by the parent in this round.
- Parent accepted `Galileo`'s smaller recommendation before `Harvey`'s broader engine snapshot extraction to keep this round low-risk and non-PBR.
- Previous round's runtime PBR verification config include boundary cleanup was parent-owned and had no delegated write scope.
- Previous round's runtime application public header include boundary cleanup was parent-owned and had no delegated write scope.
- Previous round's runtime frame lifecycle types compatibility aggregator removal was parent-owned and had no delegated write scope.
- Previous round's runtime engine lifecycle state/cleanup refs header split was parent-owned and had no delegated write scope.
- Previous round's runtime camera/scene prepare config consumer header forward boundary cleanup was parent-owned and had no delegated write scope.
- Previous round's runtime PBR verification args compatibility facade removal was parent-owned and had no delegated write scope.
- Previous round's runtime verification args public header config forward boundary cleanup was parent-owned and had no delegated write scope.
- Previous round's runtime application config include surface follow-up cleanup implemented the `Huygens` recommendation and had no delegated write scope.
- Previous round's editor startup state parameter and callback binder include cleanup was parent-owned; `Huygens` only performed the read-only config include audit that this round implements.
- Completed read-only sidecar subagent `Huygens` audited `RuntimeApplicationConfig` include surface, changed no files, found no blocker, and recommended a narrow follow-up: remove redundant `RuntimeApplicationConfig.h` from `RuntimeApplicationFrameEditorCallbackBridge.cpp` and optionally forward-declare `RuntimeApplicationShellConfig` in `RuntimeApplicationRunner.h` while keeping the full include in the `.cpp`.
- `Huygens` also confirmed `RuntimeApplicationShell.h`, `RuntimeVerificationArgs.h`, config-policy/content-policy/window/shutdown verification implementations, and `RuntimeApplicationConfig.h` aggregate member includes should keep their full config dependencies because they store by value, return by value, read fields, or need aggregate completeness.
- Previous round's runtime verification lifecycle facade removal was parent-owned and had no delegated write scope.
- Previous read-only sidecar subagent `Newton` audited the runtime context owner boundary, changed no files, confirmed the accessor/owner direction, identified all field-style runtime context call sites, and confirmed no project registration change was required.
- Previous round's shutdown verification bridge cleanup lifecycle direct dependency cleanup was parent-owned and had no delegated write scope; `/subagents` tool availability was rechecked there.
- Previous round's runtime frame lifecycle config/state header split was parent-owned and had no disjoint sidecar write scope.
- Previous round's runtime frame clock config header extraction was parent-owned and had no disjoint sidecar write scope.
- Previous round's runtime window lifecycle types header extraction was parent-owned and had no disjoint sidecar write scope.
- Previous round's runtime renderer backend keys header extraction was parent-owned and had no disjoint sidecar write scope.
- Previous round's renderer backend registry types header extraction was parent-owned after `Sartre` completed a read-only advisory audit.
- Previous round's renderer backend key string-view boundary cleanup was parent-owned and had no disjoint sidecar write scope.
- Previous round's Engine lifecycle header type include surface cleanup was parent-owned and had no disjoint sidecar write scope.
- Previous round's shutdown cleanup refs canonicalization was parent-owned and had no disjoint sidecar write scope.
- Previous round's content camera/scene sub-lifecycle header cleanup was parent-owned and had no disjoint sidecar write scope.
- Previous round's content lifecycle header config cleanup was parent-owned and had no disjoint sidecar write scope.
- Previous round's content config policy header include cleanup was parent-owned and had no disjoint sidecar write scope.
- Previous round's application config policy header include cleanup was parent-owned and had no disjoint sidecar write scope.
- Previous round's application startup bridge include surface cleanup was parent-owned and had no disjoint sidecar write scope.
- Previous round's application shutdown bridge implementation include cleanup was parent-owned and had no disjoint sidecar write scope.
- Previous round's application frame bridge implementation include cleanup was parent-owned and had no disjoint sidecar write scope.
- Previous round's frame runner types header extraction was parent-owned and had no disjoint sidecar write scope.
- Previous round's application frame editor callback bridge header cleanup followed `Ptolemy`'s recommendation and was parent-owned.
- Completed read-only sidecar subagent `Ptolemy` audited the next safe frame header boundary, changed no files, recommended the smaller application frame editor callback bridge header cleanup before `RuntimeFrameRunnerTypes.h`, and reported focused/full verification expectations.
- Parent accepted `Ptolemy`'s minimum-write-scope recommendation and implemented the bridge header cleanup locally; no sidecar has write ownership for this slice.
- Previous round's frame lifecycle types header extraction was parent-owned and had no disjoint sidecar write scope.
- Previous round's frame callbacks header extraction was parent-owned and had no disjoint sidecar write scope.
- Previous round's Runtime Verification Lifecycle header forward boundary cleanup was parent-owned and had no disjoint sidecar write scope.
- Previous round's stop policy extraction was parent-owned and had no disjoint sidecar write scope.
- Previous round's startup profile lifecycle extraction was parent-owned and had no disjoint sidecar write scope.
- Previous round's cleanup lifecycle extraction was parent-owned and had no disjoint sidecar write scope.
- No new sidecar subagent was started in this round; the cleanup lifecycle extraction is parent-owned and has no disjoint sidecar write scope.
- Previous round's prepared-scene lifecycle extraction was parent-owned after read-only sidecar audit and had no disjoint sidecar write scope.
- Completed read-only sidecar subagent `Mendel` audited the prepared-scene lifecycle extraction boundary, changed no files, confirmed this is a low-risk split, and reported that call/output order, VS project/filter registration, and verification parser coverage must remain stable.
- Parent accepted `Mendel`'s boundary constraints and implemented the prepared-scene lifecycle extraction locally; no sidecar has write ownership for this slice.
- No active sidecar remains open in this round.
- Previous round's verification frame capture lifecycle extraction was parent-owned and had no disjoint sidecar write scope.
- No new sidecar subagent was started in this round; the Engine verification report formatter extraction is parent-owned and has no disjoint sidecar write scope.
- Completed read-only sidecar subagent `James` audited the formatter extraction boundary, changed no files, confirmed the slice stays within Engine runtime verification/reporting instead of PBR expansion, and reported stable output prefixes/fields plus VS project registration requirements that the parent integrated.
- No active sidecar remains open in this round.
- Previous round's verification output cleanup was parent-owned and had no disjoint sidecar write scope.
- Previous round's stats internal compatibility cleanup was parent-owned and had no disjoint sidecar write scope.
- No new sidecar subagent was started in this round; the stats internal compatibility cleanup is parent-owned and has no disjoint sidecar write scope.
- Previous round's public compatibility API removal was parent-owned and had no disjoint sidecar write scope.
- No new sidecar subagent was started in this round; the public compatibility API removal is parent-owned and has no disjoint sidecar write scope.
- Previous round's renderer backend API naming cleanup was parent-owned and had no disjoint sidecar write scope.
- Previous round's frame execution bridge split was parent-owned and had no disjoint sidecar write scope.
- Previous round's backend slot snapshot split was parent-owned, followed the previous `Euler` follow-up, and had no disjoint sidecar write scope.
- Previous read-only sidecar subagent `Euler` audited the `RendererSubsystemBackendSlot` boundary, changed no files, confirmed attach/detach counters, attachment desc normalization, cleanup detach semantics, no-op backend mode, and VS registration had no blocking issue, and reported a low-risk unused `clear()` API footgun that the parent removed.
- Previous round's RendererSubsystem frame bridge state split was parent-owned and had no disjoint sidecar write scope.
- Previous read-only sidecar subagent `Ramanujan` audited the RendererSubsystem frame bridge stats header extraction, changed no files, confirmed the split was low risk, and reported active round documentation drift that the parent integrated.
- No sidecar subagent has write ownership in this round; source/project/documentation edits remain parent-owned.
- Previous round's legacy experiment runner implementation split was parent-owned and had no disjoint sidecar write scope.
- Previous round's renderer backend attachment lifecycle split was parent-owned and had no disjoint sidecar write scope.
- No new sidecar subagent was started in this round; the runtime engine attachment split is parent-owned and has no disjoint sidecar write scope.
- No new sidecar subagent was started in this round; the runtime camera/light split is parent-owned and has no disjoint sidecar write scope.
- No new sidecar subagent was started in this round; the runtime render resource split is parent-owned and has no disjoint sidecar write scope.
- Read-only sidecar subagent `Bacon` audited this round's Runtime Profile State extraction, changed no files, confirmed the source boundary and VS registration, and reported stale `work.md` direction lines that the parent integrated.
- Previous sidecar subagent `Mill` was started for the Runtime Engine Lifecycle Types Header audit but timed out before returning findings; it was closed with no file edits or integrated findings.
- No new sidecar subagent was started in this round; the continue/run split followed the previous `Lorentz` recommendation and stayed within parent-owned application frame files.
- After the user's `/subagents` direction, read-only sidecar subagent `Lorentz` audited the next safe continuation slice, changed no files, and recommended frame editor callback bridge as the smallest low-risk follow-up.
- Parent accepted `Lorentz`'s recommendation and implemented the frame editor callback bridge locally; no sidecar has write ownership for this slice.
- Future sidecars must use this document as the shared communication boundary, must not edit files unless explicitly assigned a disjoint write scope, and must report findings in the shared communication format.
- Read-only sidecar subagent `Jason` audited this round's frame startup lifecycle extraction, changed no files, found no blocking source issue, confirmed startup order and project registration, and noted only that startup order is covered by static review rather than a dedicated automated assertion.
- Read-only sidecar subagent `Darwin` audited this round's legacy experiment lifecycle extraction, changed no files, found no blocking source issue, and reported stale docs that the parent integrated.
- Read-only sidecar subagent `McClintock` audited renderer backend lifecycle extraction, changed no files, found no blocking source issue, and reported stale docs that the parent integrated.
- Parent continues local implementation and verification without waiting because the sidecar audit is not on the critical path.
- Read-only sidecar subagent `Bohr` audited documentation consistency for the light/camera extraction; it changed no files and reported stale current-direction wording that the parent integrated.
- Previous read-only sidecar subagent `Kuhn` audited the pass-profile extraction; it changed no files and only reported documentation cleanup that the parent integrated.
- Previous read-only sidecar subagent `Avicenna` audited the config pass/probe/deferred split; it changed no files and ran no tests.
- Previous read-only sidecar subagent `Lagrange` audited subagent coordination and documentation update scope in the prior round; it changed no files and ran no tests.
- Previous read-only extraction/args/docs audit agents have been closed; their duplicate-symbol and documentation-overclaim warnings remain reflected in the staged report extraction.
- Subagents must not edit files in this round unless the parent explicitly assigns a disjoint write scope.
- No new sidecar subagent was started in this round; the Runtime render resource frame render targets owner boundary cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeRenderResourceState.h`, `application/RuntimeRenderResourceState.cpp`, `application/RuntimeFramePasses.cpp`, `application/RuntimeRendererFrameBridgeAdapter.cpp`, `application/RuntimeSceneSetupContextFactory.cpp`, `application/RuntimeWindowLifecycle.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime profile state camera rig owner boundary cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeProfileState.h`, `application/RuntimeProfileState.cpp`, `application/RuntimeProfileLoader.cpp`, `application/RuntimePBRLightCameraRigVerification.cpp`, `application/RuntimeEditorPanelCoordinator.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime profile state environment profile owner boundary cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeProfileState.h`, `application/RuntimeProfileState.cpp`, `application/RuntimeProfileLoader.cpp`, `application/RuntimePBRStartupProfileVerification.cpp`, `application/RuntimeSceneSetupContextFactory.cpp`, `application/RuntimeEditorPanelCoordinator.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime profile state post-process settings owner boundary cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeProfileState.h`, `application/RuntimeProfileState.cpp`, `application/RuntimeProfileLoader.cpp`, `application/RuntimePBRStartupProfileVerification.cpp`, `application/RuntimeFramePasses.cpp`, `application/RuntimeRendererFrameBridgeAdapter.cpp`, `application/RuntimeEditorPanelCoordinator.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime profile state light rig owner boundary cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeProfileState.h`, `application/RuntimeProfileState.cpp`, `application/RuntimePBRLightCameraRigVerification.cpp`, `application/RuntimeProfileLoader.cpp`, `application/RuntimeSceneSetupContextFactory.cpp`, `application/RuntimeEditorPanelCoordinator.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime profile state preview profile owner boundary cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeProfileState.h`, `application/RuntimeProfileState.cpp`, `application/RuntimePBRPreviewProfileVerification.cpp`, `application/RuntimeProfileLoader.cpp`, `application/RuntimeSceneSetupContextFactory.cpp`, `application/RuntimeEditorPanelCoordinator.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime profile state frame pipeline profile owner boundary cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeProfileState.h`, `application/RuntimeProfileState.cpp`, `application/RuntimeEditorPanelCoordinator.cpp`, `application/RuntimeFramePipeline.cpp`, `application/RuntimeFramePassRegistry.cpp`, `application/RuntimePBRStartupProfileVerification.cpp`, `application/RuntimeProfileLoader.cpp`, `application/RuntimeRendererFrameBridgeAdapter.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime render resource Bloom owner boundary cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeRenderResourceState.h`, `application/RuntimeRenderResourceState.cpp`, `application/RuntimeSceneSetupContextFactory.cpp`, `application/RuntimeFramePasses.cpp`, `application/RuntimeRendererFrameBridgeAdapter.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime render resource screen quad owner boundary cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeRenderResourceState.h`, `application/RuntimeRenderResourceState.cpp`, `application/RuntimeSceneSetupContextFactory.cpp`, `application/RuntimeFramePasses.cpp`, `application/RuntimeRendererFrameBridgeAdapter.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime render resource screen material owner boundary cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeRenderResourceState.h`, `application/RuntimeRenderResourceState.cpp`, `application/RuntimeSceneSetupContextFactory.cpp`, `application/RuntimeWindowLifecycle.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime render resource scene mesh/material owner boundary cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeRenderResourceState.h`, `application/RuntimeRenderResourceState.cpp`, `application/RuntimeSceneSetupContextFactory.cpp`, `application/RuntimeLegacyExperimentLifecycle.cpp`, `application/RuntimeEditorPanelCoordinator.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime render resource dead point light mesh owner removal is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeRenderResourceState.h`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime render resource renderer/scene owner boundary cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeRenderResourceState.h`, `application/RuntimeRenderResourceState.cpp`, `application/RuntimeContentRendererBackendLifecycle.cpp`, `application/RuntimeEditorPanelCoordinator.cpp`, `application/RuntimeEngineWorldVerification.cpp`, `application/RuntimeFramePasses.cpp`, `application/RuntimeFrameRunner.cpp`, `application/RuntimeImportedAssetVerification.cpp`, `application/RuntimeLegacyExperimentLifecycle.cpp`, `application/RuntimePBRPassProfileVerification.cpp`, `application/RuntimePBRPreparedSceneStatsVerification.cpp`, `application/RuntimePBRRendererStatsVerification.cpp`, `application/RuntimePBRSceneProbeVerification.cpp`, `application/RuntimeProfileLoader.cpp`, `application/RuntimeRendererBackendAttachmentLifecycle.cpp`, `application/RuntimeRendererFrameBridgeAdapter.cpp`, `application/RuntimeSceneSetupContextFactory.cpp`, `application/RuntimeVerificationReport.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime render resource clear color state boundary cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeRenderResourceState.h`, `application/RuntimeRenderResourceState.cpp`, `application/RuntimeFrameRunner.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime render resource read-only view facade cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeRenderResourceState.h`, `application/RuntimeRenderResourceState.cpp`, `application/RuntimeContentRendererBackendLifecycle.cpp`, `application/RuntimeRendererBackendAttachmentLifecycle.cpp`, `application/RuntimeRendererFrameBridgeAdapter.cpp`, `application/RuntimeVerificationReport.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime render resource read-only view PBR stats consumer cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimePBRRendererStatsVerification.cpp`, `application/RuntimePBRPreparedSceneStatsVerification.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime render resource renderer pass profile access boundary cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeRenderResourceState.h`, `application/RuntimeRenderResourceState.cpp`, `application/RuntimePBRPassProfileVerification.cpp`, `application/RuntimeProfileLoader.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime render resource renderer clear color sync boundary cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeRenderResourceState.h`, `application/RuntimeRenderResourceState.cpp`, `application/RuntimeFrameRunner.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime render resource PBR scene probe boundary cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeRenderResourceState.h`, `application/RuntimeRenderResourceState.cpp`, `application/RuntimePBRSceneProbeVerification.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime render resource imported asset probe scene boundary cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeImportedAssetVerification.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime asset import service adapter cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeAssetImportService.h`, `application/RuntimeAssetImportService.cpp`, `application/RuntimeImportedAssetVerification.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime engine world verification read-only resource cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeEngineWorldVerification.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime frame render resource adapter cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeFrameRenderResourceAdapter.h`, `application/RuntimeFrameRenderResourceAdapter.cpp`, `application/RuntimeFramePasses.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime editor render resource adapter cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeEditorRenderResourceAdapter.h`, `application/RuntimeEditorRenderResourceAdapter.cpp`, `application/RuntimeEditorPanelCoordinator.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime scene setup resource adapter cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeSceneSetupResourceAdapter.h`, `application/RuntimeSceneSetupResourceAdapter.cpp`, `application/RuntimeSceneSetupContextFactory.cpp`, `application/RuntimeLegacyExperimentLifecycle.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime window resize resource adapter cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeWindowRenderResourceAdapter.h`, `application/RuntimeWindowRenderResourceAdapter.cpp`, `application/RuntimeWindowLifecycle.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime renderer backend resource adapter cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeRendererBackendResourceAdapter.h`, `application/RuntimeRendererBackendResourceAdapter.cpp`, `application/RuntimeContentRendererBackendLifecycle.cpp`, `application/RuntimeRendererBackendAttachmentLifecycle.cpp`, `application/RuntimeVerificationReport.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime PBR stats resource adapter cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimePBRStatsResourceAdapter.h`, `application/RuntimePBRStatsResourceAdapter.cpp`, `application/RuntimePBRRendererStatsVerification.cpp`, `application/RuntimePBRPreparedSceneStatsVerification.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime frame readiness resource adapter cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeFrameReadinessResourceAdapter.h`, `application/RuntimeFrameReadinessResourceAdapter.cpp`, `application/RuntimeRendererFrameBridgeAdapter.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime engine world verification resource adapter cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeEngineWorldVerificationResourceAdapter.h`, `application/RuntimeEngineWorldVerificationResourceAdapter.cpp`, `application/RuntimeEngineWorldVerification.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime probe scene resource adapter cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeProbeSceneResourceAdapter.h`, `application/RuntimeProbeSceneResourceAdapter.cpp`, `application/RuntimePBRSceneProbeVerification.cpp`, `application/RuntimeImportedAssetVerification.cpp`, `application/RuntimeRenderResourceState.h`, `application/RuntimeRenderResourceState.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime renderer state resource adapter cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeRendererStateResourceAdapter.h`, `application/RuntimeRendererStateResourceAdapter.cpp`, `application/RuntimeFrameRunner.cpp`, `application/RuntimePBRPassProfileVerification.cpp`, `application/RuntimeProfileLoader.cpp`, `application/RuntimeRenderResourceState.h`, `application/RuntimeRenderResourceState.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime render resource read-only view facade removal is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeFrameReadinessResourceAdapter.cpp`, `application/RuntimeRenderResourceState.h`, `application/RuntimeRenderResourceState.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime frame pass registry profile predicate cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeFramePassRegistry.h`, `application/RuntimeFramePassRegistry.cpp`, `application/RuntimeFramePipeline.cpp`, `application/RuntimeRendererFrameBridgeAdapter.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime inspector implementation split is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/inspector/PropertyInspector.h`, `tools/inspector/PropertyInspector.cpp`, `tools/inspector/MaterialInspector.h`, `tools/inspector/MaterialInspector.cpp`, material property schema implementation files, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime scene object inspector schema cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/inspector/SceneObjectInspector.h`, `tools/inspector/SceneObjectInspector.cpp`, `tools/inspector/PropertySchema.h`, `tools/inspector/PropertyInspector.cpp`, `tools/config/ProfileConfigIO.cpp`, `tools/editor/EditorPanels.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime legacy object transform inspector schema cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/inspector/SceneObjectInspector.h`, `tools/inspector/SceneObjectInspector.cpp`, `tools/inspector/PropertySchema.h`, `tools/inspector/PropertyInspector.cpp`, `tools/config/ProfileConfigIO.cpp`, `tools/editor/EditorPanels.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime engine world inspector schema extraction is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/inspector/EngineWorldInspector.h`, `tools/inspector/EngineWorldInspector.cpp`, `tools/editor/EditorPanels.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime asset inspector schema extraction is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/inspector/AssetInspector.h`, `tools/inspector/AssetInspector.cpp`, `tools/editor/EditorPanels.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime selection inspector panel extraction is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/inspector/SelectionInspectorPanel.cpp`, `tools/inspector/SceneObjectInspector.h`, `tools/inspector/SceneObjectInspector.cpp`, `tools/editor/EditorPanels.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime hierarchy and asset browser panel extraction is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/editor/HierarchyPanel.cpp`, `tools/editor/AssetBrowserPanel.cpp`, `tools/editor/EditorPanels.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime editor panel header boundary split is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/editor/EditorPanelContext.h`, `tools/editor/EditorPanelFacades.h`, `tools/editor/EditorPanels.h`, `tools/editor/EditorPanels.cpp`, `tools/editor/HierarchyPanel.cpp`, `tools/editor/AssetBrowserPanel.cpp`, `tools/inspector/SelectionInspectorPanel.cpp`, `application/RuntimeEditorPanelCoordinator.cpp`, `application/RuntimeEditorRenderResourceAdapter.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime selection inspector provider registry is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/inspector/SelectionInspectorProviderRegistry.h`, `tools/inspector/SelectionInspectorProviderRegistry.cpp`, `tools/inspector/SelectionInspectorPanel.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime selection inspector provider factory extraction is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/inspector/SelectionInspectorProviders.h`, `tools/inspector/SelectionInspectorProviders.cpp`, `tools/inspector/SelectionInspectorPanel.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime ActorComponent property provider registry is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/inspector/ActorComponentPropertyProviderRegistry.h`, `tools/inspector/ActorComponentPropertyProviderRegistry.cpp`, `tools/inspector/ActorComponentPropertyProviders.h`, `tools/inspector/ActorComponentPropertyProviders.cpp`, `tools/inspector/EngineWorldInspector.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime Actor property provider registry is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/inspector/ActorPropertyProviderRegistry.h`, `tools/inspector/ActorPropertyProviderRegistry.cpp`, `tools/inspector/ActorPropertyProviders.h`, `tools/inspector/ActorPropertyProviders.cpp`, `tools/inspector/EngineWorldInspector.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime Material property provider registry is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/inspector/MaterialPropertyProviderRegistry.h`, `tools/inspector/MaterialPropertyProviderRegistry.cpp`, `tools/inspector/MaterialPropertyProviders.h`, `tools/inspector/MaterialPropertyProviders.cpp`, `tools/inspector/MaterialInspector.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime Material property provider schema extraction is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/inspector/MaterialPropertyProviderRegistry.h`, `tools/inspector/MaterialPropertyProviderRegistry.cpp`, `tools/inspector/MaterialPropertyProviders.cpp`, `tools/inspector/MaterialInspector.cpp`, `materials/material.h`, `materials/material.cpp`, `materials/phongMaterial.h`, `materials/phongMaterial.cpp`, `materials/phongCSMShadowMaterial/phongCSMShadowMaterial.h`, `materials/phongCSMShadowMaterial/phongCSMShadowMaterial.cpp`, `materials/phongPointShadowMaterial/phongPointShadowMaterial.h`, `materials/phongPointShadowMaterial/phongPointShadowMaterial.cpp`, `materials/grassInstanceMaterial/grassInstanceMaterial.h`, `materials/grassInstanceMaterial/grassInstanceMaterial.cpp`, `materials/screenMaterial.h`, `materials/screenMaterial.cpp`, `materials/pbrMaterial/PBRMaterial.h`, `materials/pbrMaterial/PBRMaterial.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime Material editable accessor boundary is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/inspector/MaterialPropertyProviders.cpp`, `materials/phongMaterial.h`, `materials/phongMaterial.cpp`, `materials/phongCSMShadowMaterial/phongCSMShadowMaterial.h`, `materials/phongCSMShadowMaterial/phongCSMShadowMaterial.cpp`, `materials/phongPointShadowMaterial/phongPointShadowMaterial.h`, `materials/phongPointShadowMaterial/phongPointShadowMaterial.cpp`, `materials/grassInstanceMaterial/grassInstanceMaterial.h`, `materials/grassInstanceMaterial/grassInstanceMaterial.cpp`, `materials/screenMaterial.h`, `materials/screenMaterial.cpp`, `materials/pbrMaterial/PBRMaterial.h`, `materials/pbrMaterial/PBRMaterial.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime Material edit controls DTO cleanup is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `materials/MaterialEditControls.h`, `tools/inspector/MaterialPropertyProviders.cpp`, `materials/phongMaterial.h`, `materials/phongMaterial.cpp`, `materials/phongCSMShadowMaterial/phongCSMShadowMaterial.h`, `materials/phongCSMShadowMaterial/phongCSMShadowMaterial.cpp`, `materials/phongPointShadowMaterial/phongPointShadowMaterial.h`, `materials/phongPointShadowMaterial/phongPointShadowMaterial.cpp`, `materials/grassInstanceMaterial/grassInstanceMaterial.h`, `materials/grassInstanceMaterial/grassInstanceMaterial.cpp`, `materials/screenMaterial.h`, `materials/screenMaterial.cpp`, `materials/pbrMaterial/PBRMaterial.h`, `materials/pbrMaterial/PBRMaterial.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Screen material input texture encapsulation is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `materials/screenMaterial.h`, `materials/screenMaterial.cpp`, `renderer/PostProcessPass.cpp`, `tools/sceneSetup/SceneSetup.cpp`, `application/RuntimeViewport.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Phong surface runtime state encapsulation is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `materials/MaterialEditControls.h`, `materials/phongMaterial.h`, `materials/phongMaterial.cpp`, `materials/phongPointShadowMaterial/phongPointShadowMaterial.h`, `materials/phongPointShadowMaterial/phongPointShadowMaterial.cpp`, `materials/phongCSMShadowMaterial/phongCSMShadowMaterial.h`, `materials/phongCSMShadowMaterial/phongCSMShadowMaterial.cpp`, `renderer/MaterialBinder.cpp`, `tools/sceneSetup/SceneSetup.cpp`, `tools/legacyExperiments/LegacyExperimentRunner.cpp`, `application/AssimpMaterialImporter.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Grass surface runtime state encapsulation is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `materials/MaterialEditControls.h`, `materials/grassInstanceMaterial/grassInstanceMaterial.h`, `materials/grassInstanceMaterial/grassInstanceMaterial.cpp`, `renderer/MaterialBinder.cpp`, `tools/legacyExperiments/LegacyExperimentRunner.cpp`, `application/assimpInstanceLoader.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the PBR material runtime state API bridge is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `materials/MaterialEditControls.h`, `materials/pbrMaterial/PBRMaterial.h`, `materials/pbrMaterial/PBRMaterial.cpp`, `renderer/PBRSurfaceResourceBinder.cpp`, `renderer/PBRAlphaShadowBinder.cpp`, `renderer/PBRDepthPrepass.cpp`, `renderer/PBRIBLResourceBinder.cpp`, `renderer/PBRGBufferPass.cpp`, `application/RuntimePBRStatsResourceAdapter.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the PBR material private field encapsulation is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `materials/pbrMaterial/PBRMaterial.h`, `materials/pbrMaterial/PBRMaterial.cpp`, `application/AssimpMaterialImporter.cpp`, `application/RuntimeEngineWorldVerificationResourceAdapter.cpp`, `application/RuntimePBRSceneProbeVerification.cpp`, `tools/sceneSetup/SceneSetup.cpp`, `tools/sceneSetup/WorldDrivenSceneSetup.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the PBR material profile config schema adapter is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `materials/pbrMaterial/PBRMaterial.cpp`, `materials/pbrMaterial/PBRMaterialProfile.h`, `materials/pbrMaterial/PBRMaterialProfile.cpp`, `materials/pbrMaterial/PBRMaterialProfileConfig.h`, `materials/pbrMaterial/PBRMaterialProfileConfig.cpp`, `tools/sceneSetup/PBRPreviewProfile.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Post Process settings config schema adapter is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `renderer/PostProcessSettings.h`, `renderer/PostProcessSettings.cpp`, `renderer/PostProcessSettingsConfig.h`, `renderer/PostProcessSettingsConfig.cpp`, `tools/editor/DebugControllerPanel.cpp`, `tools/sceneSetup/PBRExperimentProfile.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Environment profile config schema adapter is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `renderer/EnvironmentProfile.h`, `renderer/EnvironmentProfile.cpp`, `renderer/EnvironmentProfileConfig.h`, `renderer/EnvironmentProfileConfig.cpp`, `tools/editor/DebugControllerPanel.cpp`, `tools/sceneSetup/PBRExperimentProfile.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Renderer frame pass profile config schema adapter is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `renderer/RendererFramePassProfile.h`, `renderer/RendererFramePassProfile.cpp`, `renderer/RendererFramePassProfileConfig.h`, `renderer/RendererFramePassProfileConfig.cpp`, `tools/editor/DebugControllerPanel.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the PBR preview profile config schema adapter is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/sceneSetup/PBRPreviewProfile.h`, `tools/sceneSetup/PBRPreviewProfile.cpp`, `tools/sceneSetup/PBRPreviewProfileConfig.h`, `tools/sceneSetup/PBRPreviewProfileConfig.cpp`, `tools/sceneSetup/PBRExperimentProfile.cpp`, `tools/editor/DebugControllerPanel.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the PBR Light/Camera rig config schema adapter is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/sceneSetup/PBRLightRigProfile.h`, `tools/sceneSetup/PBRLightRigProfile.cpp`, `tools/sceneSetup/PBRLightRigProfileConfig.h`, `tools/sceneSetup/PBRLightRigProfileConfig.cpp`, `tools/sceneSetup/PBRCameraRigProfile.h`, `tools/sceneSetup/PBRCameraRigProfile.cpp`, `tools/sceneSetup/PBRCameraRigProfileConfig.h`, `tools/sceneSetup/PBRCameraRigProfileConfig.cpp`, `tools/sceneSetup/PBRExperimentProfile.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime frame pipeline profile config schema adapter is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeFramePipelineProfile.h`, `application/RuntimeFramePipelineProfile.cpp`, `application/RuntimeFramePipelineProfileConfig.h`, `application/RuntimeFramePipelineProfileConfig.cpp`, `tools/editor/DebugControllerPanel.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Debug profile controls panel extraction is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/editor/DebugControllerPanel.cpp`, `tools/editor/DebugProfileControlsPanel.h`, `tools/editor/DebugProfileControlsPanel.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Debug controller remaining panel extraction is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/editor/DebugControllerPanel.cpp`, `tools/editor/DebugLegacyControlsPanel.h`, `tools/editor/DebugLegacyControlsPanel.cpp`, `tools/editor/RendererFrameStatsPanel.h`, `tools/editor/RendererFrameStatsPanel.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Debug controller section registry is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/editor/DebugControllerPanel.cpp`, `tools/editor/DebugControllerSectionRegistry.h`, `tools/editor/DebugControllerSectionRegistry.cpp`, `tools/editor/DebugControllerSections.h`, `tools/editor/DebugControllerSections.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Debug profile control section registry is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/editor/DebugProfileControlsPanel.cpp`, `tools/editor/DebugProfileControlSectionRegistry.h`, `tools/editor/DebugProfileControlSectionRegistry.cpp`, `tools/editor/DebugProfileControlSections.h`, `tools/editor/DebugProfileControlSections.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Keyed section registry extraction is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/editor/KeyedSectionRegistry.h`, `tools/editor/DebugControllerSectionRegistry.h`, deleted `tools/editor/DebugControllerSectionRegistry.cpp`, `tools/editor/DebugProfileControlSectionRegistry.h`, deleted `tools/editor/DebugProfileControlSectionRegistry.cpp`, `tools/editor/DebugControllerSections.h`, `tools/editor/DebugProfileControlSections.h`, `tools/editor/DebugControllerPanel.cpp`, `tools/editor/DebugProfileControlsPanel.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Keyed section ordering and diagnostics slice is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/editor/KeyedSectionRegistry.h`, `tools/editor/DebugControllerSectionRegistry.h`, `tools/editor/DebugProfileControlSectionRegistry.h`, `tools/editor/DebugControllerSections.cpp`, `tools/editor/DebugProfileControlSections.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Debug pipeline profile control provider extraction is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/editor/DebugPipelineProfileControlSections.h`, `tools/editor/DebugPipelineProfileControlSections.cpp`, `tools/editor/DebugProfileControlSections.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Debug scene profile control provider extraction is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/editor/DebugSceneProfileControlSections.h`, `tools/editor/DebugSceneProfileControlSections.cpp`, `tools/editor/DebugProfileControlSections.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Debug section registration helper extraction is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/editor/DebugSectionRegistration.h`, `tools/editor/KeyedSectionRegistry.h`, `tools/editor/DebugControllerSections.cpp`, `tools/editor/DebugPipelineProfileControlSections.cpp`, `tools/editor/DebugSceneProfileControlSections.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Debug controller section registration entry is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/editor/DebugControllerSections.h`, `tools/editor/DebugControllerSections.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Editor UI module registry composer is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/editor/EditorUiModuleRegistry.h`, `tools/editor/EditorUiModuleRegistry.cpp`, `tools/editor/DebugControllerPanel.cpp`, `tools/editor/DebugProfileControlsPanel.cpp`, `tools/inspector/SelectionInspectorPanel.cpp`, `tools/inspector/SelectionInspectorProviders.h`, `tools/inspector/SelectionInspectorProviders.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Editor UI module registration list is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/editor/EditorUiModuleRegistry.h`, `tools/editor/EditorUiModuleRegistry.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the sample editor UI module is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/editor/SampleEditorUiModule.h`, `tools/editor/SampleEditorUiModule.cpp`, `tools/editor/EditorUiModuleRegistry.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Editor UI module composition policy slice is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/editor/EditorUiModuleComposition.h`, `tools/editor/EditorUiModuleComposition.cpp`, `tools/editor/EditorUiModuleRegistry.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime Editor UI module state injection slice is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeEditorLifecycle.cpp`, `application/RuntimeEditorLifecycleState.h`, `application/RuntimeEditorLifecycleState.cpp`, `application/RuntimeEditorPanelCoordinator.h`, `application/RuntimeEditorPanelCoordinator.cpp`, `tools/editor/DebugControllerContext.h`, `tools/editor/DebugControllerPanel.cpp`, `tools/editor/DebugProfileControlsPanel.cpp`, `tools/editor/EditorPanelContext.h`, `tools/inspector/SelectionInspectorPanel.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime Editor UI module config policy slice is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeApplicationConfig.h`, `application/RuntimeApplicationConfigPolicy.cpp`, `application/RuntimeApplicationEditorStartupLifecycle.h`, `application/RuntimeApplicationEditorStartupLifecycle.cpp`, `application/RuntimeApplicationStartupLifecycle.cpp`, `application/RuntimeEditorLifecycleConfig.h`, `application/RuntimeEditorLifecycle.h`, `application/RuntimeEditorLifecycle.cpp`, `application/RuntimeEditorLifecycleState.h`, `application/RuntimeEditorLifecycleState.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime Editor UI module CLI policy slice is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeVerificationArgs.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Editor UI module diagnostics section slice is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/editor/EditorUiModuleRegistry.h`, `tools/editor/EditorUiModuleRegistry.cpp`, `tools/editor/EditorUiModuleComposition.cpp`, `tools/editor/EditorUiModuleDiagnosticsSection.h`, `tools/editor/EditorUiModuleDiagnosticsSection.cpp`, `tools/inspector/SelectionInspectorProviderRegistry.h`, `tools/inspector/SelectionInspectorProviderRegistry.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Editor UI module profile storage slice is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/editor/EditorUiModuleProfile.h`, `tools/editor/EditorUiModuleProfile.cpp`, `tools/editor/EditorUiModuleProfileConfig.h`, `tools/editor/EditorUiModuleProfileConfig.cpp`, `application/RuntimeProfileState.h`, `application/RuntimeProfileState.cpp`, `application/RuntimeProfileLoader.cpp`, `application/RuntimeApplicationConfig.h`, `application/RuntimeApplicationConfigPolicy.h`, `application/RuntimeApplicationConfigPolicy.cpp`, `application/RuntimeApplicationEditorStartupLifecycle.cpp`, `application/RuntimeVerificationArgs.cpp`, `config/editor_ui_modules.example.ini`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Editor UI module profile controls slice is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/editor/DebugControllerContext.h`, `tools/editor/EditorUiModuleDiagnosticsSection.cpp`, `application/RuntimeEditorPanelCoordinator.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Runtime Editor UI module reapply boundary slice is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `application/RuntimeEditorLifecycleState.h`, `application/RuntimeEditorLifecycleState.cpp`, `application/RuntimeEditorLifecycle.cpp`, `application/RuntimeEditorPanelCoordinator.h`, `application/RuntimeEditorPanelCoordinator.cpp`, `tools/editor/DebugControllerContext.h`, `tools/editor/EditorUiModuleDiagnosticsSection.cpp`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Editor UI module profile controls section extraction slice is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/editor/EditorUiModuleDiagnosticsSection.cpp`, `tools/editor/EditorUiModuleProfileControlsSection.h`, `tools/editor/EditorUiModuleProfileControlsSection.cpp`, `tools/editor/EditorUiModuleComposition.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.
- No new sidecar subagent was started in this round; the Editor UI module policy diagnostics slice is parent-owned and has no disjoint sidecar write scope.
- Parent-owned write scope for this round: `tools/editor/EditorUiModulePolicyDiagnostics.h`, `application/RuntimeEditorLifecycleState.h`, `application/RuntimeEditorLifecycleState.cpp`, `application/RuntimeEditorPanelCoordinator.cpp`, `tools/editor/DebugControllerContext.h`, `tools/editor/EditorUiModuleDiagnosticsSection.cpp`, `text2.vcxproj`, `text2.vcxproj.filters`, `work.md`, `worked.md`, and planning docs.

Merge rule:

- Subagent findings are advisory until parent integrates them.
- Parent must record accepted implementation work in `worked.md`.
- If a subagent recommendation conflicts with already implemented parent code, the parent resolves the conflict explicitly and records the decision in the docs.
- Parent continues implementation without waiting if the next local step is not blocked by the sidecar audit.

Shared sidecar communication format:

- `Scope checked`: exact files or modules inspected.
- `Findings`: blocking issues first, then non-blocking drift.
- `Verification risk`: expected focused/full verification impact.
- `Edits made`: must be `none` unless the parent explicitly assigned a disjoint write scope.
- `Recommendation`: accept/reject/needs-parent-review with concise reasoning.

## Parent Agent Responsibilities

- Maintain `work.md` and `worked.md`.
- Run verification commands.
- Decide which agent recommendations are implemented.
- Keep the active goal moving without redefining completion.
