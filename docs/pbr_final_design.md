# PBR Final Foundation Design

## Purpose

This document defines the last necessary PBR work before the project shifts back to engine architecture. It is a freeze contract, not a feature wishlist.

The PBR renderer only needs to be strong enough to serve as the first real renderer pipeline in the engine.

## Frozen Scope

The current PBR foundation includes:

- Forward PBR rendering.
- Deferred PBR rendering with G-buffer.
- PBR material scalar and texture binding.
- Basic IBL parameter flow.
- Directional and point shadow atlas integration.
- Alpha mask support.
- Tiled deferred light culling.
- Clustered deferred light culling probe.
- Clustered occupancy debug view.
- Deferred renderer GPU timing with delayed query readback.
- Multi-sample tiled vs clustered pressure profiling.
- Scripted verification coverage.

This is enough for an engine foundation.

## Default Policy

The stable default should remain conservative:

- Use forward PBR for simple material previews and direct asset probes.
- Use deferred PBR for renderer pipeline validation.
- Treat tiled deferred light culling as the stable fallback.
- Treat clustered deferred light culling as experimental/profiling until overflow, slicing policy, and real-scene quality are solved.
- Keep GPU timing and clustered readback disabled unless verification/profiling explicitly enables them.

## Required Final Gate

Before moving fully into engine systems, PBR must satisfy these gates:

- `tools/verify_pbr.ps1 -SkipBuild -DiscardCaptures` passes all default modes.
- `tools/verify_pbr_golden.ps1 -SkipBuild -NoLinkDebugInfo -DiscardCaptures` passes.
- `docs/pbr_golden_baselines.json` records the current real-asset baseline.
- `docs/pbr_golden_verification_report.md` records the latest golden verification result.
- `docs/pbr_light_culling_timing_report.md` and `docs/pbr_light_culling_timing_samples.csv` remain as profiling references, not default runtime requirements.

## Current Freeze Evidence

Current local gate result:

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr_golden.ps1 -SkipBuild -NoLinkDebugInfo -DiscardCaptures`: passed.
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`: passed all 28 default modes.

Golden baseline coverage:

- `import`: Assimp-imported PBR asset probe.
- `texture-set`: scalar plus texture-set PBR material probe.
- `deferred-texture-set`: texture-set material through deferred G-buffer and lighting.

This is sufficient to freeze PBR as an engine foundation pipeline. Future PBR work should be treated as renderer product work unless it is needed by engine integration.

## Accepted Limitations

These are known limitations and should not block the engine transition:

- Full glTF material parity is not complete.
- Alpha blend transparency is not a production solution.
- Clustered overflow/fallback policy is not production-ready.
- Real production asset coverage is limited.
- PBR visual correctness still needs future calibration against reference assets.
- Renderer resource lifetime can still be improved during engine module cleanup.

These limitations are explicitly deferred because solving them now would continue moving the project toward a renderer-only project.

## Renderer Boundary Contract

PBR should expose engine-facing behavior through a small boundary:

- Material input: scalar factors, texture handles, alpha mode, and shading model.
- Mesh input: vertex buffers, index buffers, tangents/normals/UVs.
- Light input: directional lights, point lights, shadow metadata.
- Camera input: view/projection and exposure-relevant state.
- Output: color target, optional debug targets, renderer stats.

The engine should not depend on PBR implementation details such as:

- G-buffer attachment layout.
- Tiled or clustered SSBO binding indices.
- BRDF shader file structure.
- Renderer pass order internals.
- Verification-only flags.

## Stop Conditions

After the final gate passes, avoid adding PBR features unless one of these is true:

- A bug breaks existing verification.
- An engine interface cannot be designed without a small renderer clarification.
- A future scene/asset/editor task needs a minimal PBR hook to prove integration.

Otherwise, move the next work item to engine core, scene, asset, serialization, or editor infrastructure.
