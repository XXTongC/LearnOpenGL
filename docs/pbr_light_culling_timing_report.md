# PBR Deferred Light Culling Timing Report

生成时间：2026-05-21 22:43:06
仓库：`C:\Code\CodeOfC++\OpenGL_test\text2-refactor`
分支：`codex/text2-refactor`
提交：`bf85071`
配置：`Debug|x64`

## 测试范围

本报告只比较同一个 8 点光 pressure rig 下的 deferred PBR light culling 路径。两个模式都会启用 renderer pass 级 `GL_TIME_ELAPSED`，并使用跨帧 deferred query readback，避免为了当前帧 timing 主动阻塞 GPU。

复现命令：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools\profile_pbr_light_culling.ps1 -NoLinkDebugInfo
```

底层 verification modes：

- `deferred-tiled-lights-pressure-timing`
- `deferred-clustered-grid-pressure-timing`

## 结果

| Path | Grid | Point lights | Live indices | Culled / candidate | Occupancy | G-buffer GPU ms | Deferred lighting GPU ms | Frame GPU ms | Timed passes | Pending queries | Log |
| --- | --- | ---: | ---: | --- | --- | ---: | ---: | ---: | ---: | ---: | --- |
| Tiled | `80x45` | 8 | 8034 | 20766 / 28800 (72.10%) | 3568/3600 | 0.2273 | 1.0270 | 7.0456 | 6 | 18 | `out/pbr_verify_deferred-tiled-lights-pressure-timing.log` |
| Clustered | `80x45x24` | 8 | 8037 | 683163 / 691200 (98.84%) | 86400 clusters | 0.2650 | 2.9362 | 8.5932 | 6 | 12 | `out/pbr_verify_deferred-clustered-grid-pressure-timing.log` |

## 解释

- 本次采样中 `Tiled` 的 deferred lighting pass 更短，差值约 `1.9093 ms`。该值是当前机器和当前压力 rig 的测量结果，不应直接外推为所有场景结论。
- Tiled 路径使用屏幕 2D tile list，candidate 数量较小，适合作为当前稳定 fallback。
- Clustered 路径使用 3D grid + compute assignment，candidate 空间更大，但已经具备与深度 slice 相关的 PBR 扩展基础；后续需要继续解决 overflow/fallback、多帧采样统计和真实资产 baseline。
- 两条 timing mode 都保留在 `tools/verify_pbr.ps1` 默认回归中，避免后续重构误删 profiling 入口。

## 原始 renderer stats

### Tiled

```text
PBR verification renderer stats: rendererPasses=6, shadowCasters=32, directionalShadowLayers=5, directionalShadowDrawCalls=160, directionalAlphaMaskedShadowDrawCalls=0, pointShadowLights=8, pointShadowFaces=48, pointShadowDrawCalls=1536, pointAlphaMaskedShadowDrawCalls=0, pbrShadowAtlasReady=yes, pbrShadowAtlasDirectionalLayers=5, pbrShadowAtlasPointFaces=48, pbrShadowAtlasPointFacesRendered=48, pbrShadowAtlasDirectionalDrawCalls=160, pbrShadowAtlasPointDrawCalls=1536, pbrShadowAtlasDirectionalAlphaMaskedDrawCalls=0, pbrShadowAtlasPointAlphaMaskedDrawCalls=0, pbrDepthPrepassDrawCalls=25, legacyDrawCalls=0, legacyTransparentDrawCalls=0, pbrDrawCalls=0, pbrTransparentDrawCalls=0, rendererGpuTimingEnabled=yes, rendererGpuTimingAvailable=yes, rendererGpuTimingDeferredReadback=yes, rendererGpuTimedPasses=6, rendererGpuTimingPendingQueries=18, rendererGpuFrameNs=7045630, rendererGpuBeginFrameNs=11520, rendererGpuShadowMapsNs=2368920, rendererGpuPbrShadowAtlasNs=3324910, rendererGpuPbrDepthPrepassNs=86000, rendererGpuPbrGBufferNs=227320, rendererGpuPbrDeferredLightingNs=1026960, rendererGpuPbrDeferredTiledLightDebugNs=0, rendererGpuPbrDeferredClusteredLightDebugNs=0, rendererGpuPbrGBufferDebugNs=0, rendererGpuPbrOpaqueSceneNs=0, rendererGpuPbrTransparentSceneNs=0, pbrGBufferDrawCalls=25, pbrGBufferReady=yes, pbrGBufferSize=1280x720, pbrDeferredLightingDrawCalls=1, pbrDeferredTiledLightDebugDrawCalls=0, pbrDeferredClusteredLightDebugDrawCalls=0, pbrDeferredCsmShadowBound=yes, pbrDeferredCsmShadowLayers=5, pbrDeferredCsmShadowAtlasBound=yes, pbrDeferredPointShadowAtlasBound=yes, pbrDeferredPointShadowAtlasLights=8, pbrDeferredLightBufferBound=yes, pbrDeferredLightBufferPointLights=8/16, pbrDeferredTiledLightsEnabled=yes, pbrDeferredTiledLightGridBound=yes, pbrDeferredTiledLightGridSize=80x45, pbrDeferredTiledLightGridTileSize=16, pbrDeferredTiledLightGridCutoff=0.010000, pbrDeferredTiledLightGridPointLights=8, pbrDeferredTiledLightGridFullIndices=28800, pbrDeferredTiledLightGridIndices=8034, pbrDeferredTiledLightGridCulledIndices=20766, pbrDeferredTiledLightGridOccupiedTiles=3568/3600, pbrDeferredTiledLightGridEmptyTiles=32, pbrDeferredTiledLightGridMaxTileLights=5
```

### Clustered

```text
PBR verification renderer stats: rendererPasses=6, shadowCasters=32, directionalShadowLayers=5, directionalShadowDrawCalls=160, directionalAlphaMaskedShadowDrawCalls=0, pointShadowLights=8, pointShadowFaces=48, pointShadowDrawCalls=1536, pointAlphaMaskedShadowDrawCalls=0, pbrShadowAtlasReady=yes, pbrShadowAtlasDirectionalLayers=5, pbrShadowAtlasPointFaces=48, pbrShadowAtlasPointFacesRendered=48, pbrShadowAtlasDirectionalDrawCalls=160, pbrShadowAtlasPointDrawCalls=1536, pbrShadowAtlasDirectionalAlphaMaskedDrawCalls=0, pbrShadowAtlasPointAlphaMaskedDrawCalls=0, pbrDepthPrepassDrawCalls=25, legacyDrawCalls=0, legacyTransparentDrawCalls=0, pbrDrawCalls=0, pbrTransparentDrawCalls=0, rendererGpuTimingEnabled=yes, rendererGpuTimingAvailable=yes, rendererGpuTimingDeferredReadback=yes, rendererGpuTimedPasses=6, rendererGpuTimingPendingQueries=12, rendererGpuFrameNs=8593190, rendererGpuBeginFrameNs=14640, rendererGpuShadowMapsNs=2436720, rendererGpuPbrShadowAtlasNs=2833280, rendererGpuPbrDepthPrepassNs=107280, rendererGpuPbrGBufferNs=265040, rendererGpuPbrDeferredLightingNs=2936230, rendererGpuPbrDeferredTiledLightDebugNs=0, rendererGpuPbrDeferredClusteredLightDebugNs=0, rendererGpuPbrGBufferDebugNs=0, rendererGpuPbrOpaqueSceneNs=0, rendererGpuPbrTransparentSceneNs=0, pbrGBufferDrawCalls=25, pbrGBufferReady=yes, pbrGBufferSize=1280x720, pbrDeferredLightingDrawCalls=1, pbrDeferredTiledLightDebugDrawCalls=0, pbrDeferredClusteredLightDebugDrawCalls=0, pbrDeferredCsmShadowBound=yes, pbrDeferredCsmShadowLayers=5, pbrDeferredCsmShadowAtlasBound=yes, pbrDeferredPointShadowAtlasBound=yes, pbrDeferredPointShadowAtlasLights=8, pbrDeferredLightBufferBound=yes, pbrDeferredLightBufferPointLights=8/16, pbrDeferredTiledLightsEnabled=no, pbrDeferredTiledLightGridBound=no, pbrDeferredTiledLightGridSize=0x0, pbrDeferredTiledLightGridTileSize=0, pbrDeferredTiledLightGridCutoff=0.000000, pbrDeferredTiledLightGridPointLights=0, pbrDeferredTiledLightGridFullIndices=0, pbrDeferredTiledLightGridIndices=0, pbrDeferredTiledLightGridCulledIndices=0, pbrDeferredTiledLightGridOccupiedTiles=0/0, pbrDeferredTiledLightGridEmptyTiles=0, pbrDeferredTiledLightGridMaxTileLights=0, pbrDeferredClusteredLightGridEnabled=yes, pbrDeferredClusteredLightGridBound=yes, pbrDeferredClusteredLightGridSize=80x45x24, pbrDeferredClusteredLightGridTileSize=16, pbrDeferredClusteredLightGridClusters=86400, pbrDeferredClusteredLightGridMaxLightsPerCluster=64, pbrDeferredClusteredLightGridMaxIndices=5529600, pbrDeferredClusteredLightGridPointLights=8, pbrDeferredClusteredLightGridIndices=8037, pbrDeferredClusteredLightGridCulledIndices=683163, pbrDeferredClusteredLightGridCompute=yes, pbrDeferredClusteredLightGridStatsReadback=yes, pbrDeferredClusteredLightGridLightIndexStats=yes
```
