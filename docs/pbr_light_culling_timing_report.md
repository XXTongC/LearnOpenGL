# PBR Deferred Light Culling Timing Report

生成时间：2026-05-21 22:56:15
仓库：`C:\Code\CodeOfC++\OpenGL_test\text2-refactor`
分支：`codex/text2-refactor`
提交：`0a32120`
配置：`Debug|x64`
样本数：`3`
样本 CSV：`docs/pbr_light_culling_timing_samples.csv`

## 测试范围

本报告只比较同一个 8 点光 pressure rig 下的 deferred PBR light culling 路径。两个模式都会启用 renderer pass 级 `GL_TIME_ELAPSED`，并使用跨帧 deferred query readback，避免为了当前帧 timing 主动阻塞 GPU。

复现命令：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools\profile_pbr_light_culling.ps1 -NoLinkDebugInfo -Samples 3
```

底层 verification modes：

- `deferred-tiled-lights-pressure-timing`
- `deferred-clustered-grid-pressure-timing`

## 结果

| Path | Grid | Point lights | Live indices | Culled / candidate | Occupancy | G-buffer avg/min/max ms | Deferred lighting avg/min/max ms | Frame avg/min/max ms | Samples |
| --- | --- | ---: | ---: | --- | --- | --- | --- | --- | ---: |
| Tiled | `80x45` | 8 | 8034 | 20766 / 28800 (72.10%) | 3568/3600 | 0.2177 / 0.2042 / 0.2428 | 1.0301 / 0.9916 / 1.0529 | 6.2969 / 5.9759 / 6.7147 | 3 |
| Clustered | `80x45x24` | 8 | 8037 | 683163 / 691200 (98.84%) | 86400 clusters | 0.2329 / 0.2203 / 0.2502 | 3.4366 / 3.0088 / 3.6811 | 8.8623 / 8.3465 / 9.1491 | 3 |

## 样本

| Sample | Path | G-buffer GPU ms | Deferred lighting GPU ms | Frame GPU ms | Pending queries | Log |
| ---: | --- | ---: | ---: | ---: | ---: | --- |
| 1 | Tiled | 0.2428 | 1.0529 | 6.7147 | 18 | `out/pbr_verify_deferred-tiled-lights-pressure-timing.log` |
| 1 | Clustered | 0.2282 | 3.0088 | 8.3465 | 12 | `out/pbr_verify_deferred-clustered-grid-pressure-timing.log` |
| 2 | Tiled | 0.2042 | 1.0457 | 6.2001 | 18 | `out/pbr_verify_deferred-tiled-lights-pressure-timing.log` |
| 2 | Clustered | 0.2203 | 3.6811 | 9.0913 | 12 | `out/pbr_verify_deferred-clustered-grid-pressure-timing.log` |
| 3 | Tiled | 0.2061 | 0.9916 | 5.9759 | 18 | `out/pbr_verify_deferred-tiled-lights-pressure-timing.log` |
| 3 | Clustered | 0.2502 | 3.6200 | 9.1491 | 12 | `out/pbr_verify_deferred-clustered-grid-pressure-timing.log` |

## 解释

- 本次样本集中 `Tiled` 的 deferred lighting pass 平均时间更短，差值约 `2.4066 ms`。该值是当前机器和当前 pressure rig 的测量结果，不应直接外推为所有场景结论。
- Tiled 路径使用屏幕 2D tile list，candidate 数量较小，适合作为当前稳定 fallback。
- Clustered 路径使用 3D grid + compute assignment，candidate 空间更大，但已经具备与深度 slice 相关的 PBR 扩展基础；后续需要继续解决 overflow/fallback、多帧采样统计和真实资产 baseline。
- 两条 timing mode 都保留在 `tools/verify_pbr.ps1` 默认回归中，避免后续重构误删 profiling 入口。

## 最后一轮原始 renderer stats

### Tiled

```text
PBR verification renderer stats: rendererPasses=6, shadowCasters=32, directionalShadowLayers=5, directionalShadowDrawCalls=160, directionalAlphaMaskedShadowDrawCalls=0, pointShadowLights=8, pointShadowFaces=48, pointShadowDrawCalls=1536, pointAlphaMaskedShadowDrawCalls=0, pbrShadowAtlasReady=yes, pbrShadowAtlasDirectionalLayers=5, pbrShadowAtlasPointFaces=48, pbrShadowAtlasPointFacesRendered=48, pbrShadowAtlasDirectionalDrawCalls=160, pbrShadowAtlasPointDrawCalls=1536, pbrShadowAtlasDirectionalAlphaMaskedDrawCalls=0, pbrShadowAtlasPointAlphaMaskedDrawCalls=0, pbrDepthPrepassDrawCalls=25, legacyDrawCalls=0, legacyTransparentDrawCalls=0, pbrDrawCalls=0, pbrTransparentDrawCalls=0, rendererGpuTimingEnabled=yes, rendererGpuTimingAvailable=yes, rendererGpuTimingDeferredReadback=yes, rendererGpuTimedPasses=6, rendererGpuTimingPendingQueries=18, rendererGpuFrameNs=5975920, rendererGpuBeginFrameNs=9800, rendererGpuShadowMapsNs=2191080, rendererGpuPbrShadowAtlasNs=2506440, rendererGpuPbrDepthPrepassNs=70920, rendererGpuPbrGBufferNs=206080, rendererGpuPbrDeferredLightingNs=991600, rendererGpuPbrDeferredTiledLightDebugNs=0, rendererGpuPbrDeferredClusteredLightDebugNs=0, rendererGpuPbrGBufferDebugNs=0, rendererGpuPbrOpaqueSceneNs=0, rendererGpuPbrTransparentSceneNs=0, pbrGBufferDrawCalls=25, pbrGBufferReady=yes, pbrGBufferSize=1280x720, pbrDeferredLightingDrawCalls=1, pbrDeferredTiledLightDebugDrawCalls=0, pbrDeferredClusteredLightDebugDrawCalls=0, pbrDeferredCsmShadowBound=yes, pbrDeferredCsmShadowLayers=5, pbrDeferredCsmShadowAtlasBound=yes, pbrDeferredPointShadowAtlasBound=yes, pbrDeferredPointShadowAtlasLights=8, pbrDeferredLightBufferBound=yes, pbrDeferredLightBufferPointLights=8/16, pbrDeferredTiledLightsEnabled=yes, pbrDeferredTiledLightGridBound=yes, pbrDeferredTiledLightGridSize=80x45, pbrDeferredTiledLightGridTileSize=16, pbrDeferredTiledLightGridCutoff=0.010000, pbrDeferredTiledLightGridPointLights=8, pbrDeferredTiledLightGridFullIndices=28800, pbrDeferredTiledLightGridIndices=8034, pbrDeferredTiledLightGridCulledIndices=20766, pbrDeferredTiledLightGridOccupiedTiles=3568/3600, pbrDeferredTiledLightGridEmptyTiles=32, pbrDeferredTiledLightGridMaxTileLights=5
```

### Clustered

```text
PBR verification renderer stats: rendererPasses=6, shadowCasters=32, directionalShadowLayers=5, directionalShadowDrawCalls=160, directionalAlphaMaskedShadowDrawCalls=0, pointShadowLights=8, pointShadowFaces=48, pointShadowDrawCalls=1536, pointAlphaMaskedShadowDrawCalls=0, pbrShadowAtlasReady=yes, pbrShadowAtlasDirectionalLayers=5, pbrShadowAtlasPointFaces=48, pbrShadowAtlasPointFacesRendered=48, pbrShadowAtlasDirectionalDrawCalls=160, pbrShadowAtlasPointDrawCalls=1536, pbrShadowAtlasDirectionalAlphaMaskedDrawCalls=0, pbrShadowAtlasPointAlphaMaskedDrawCalls=0, pbrDepthPrepassDrawCalls=25, legacyDrawCalls=0, legacyTransparentDrawCalls=0, pbrDrawCalls=0, pbrTransparentDrawCalls=0, rendererGpuTimingEnabled=yes, rendererGpuTimingAvailable=yes, rendererGpuTimingDeferredReadback=yes, rendererGpuTimedPasses=6, rendererGpuTimingPendingQueries=12, rendererGpuFrameNs=9149110, rendererGpuBeginFrameNs=14320, rendererGpuShadowMapsNs=2308080, rendererGpuPbrShadowAtlasNs=2866360, rendererGpuPbrDepthPrepassNs=90120, rendererGpuPbrGBufferNs=250200, rendererGpuPbrDeferredLightingNs=3620030, rendererGpuPbrDeferredTiledLightDebugNs=0, rendererGpuPbrDeferredClusteredLightDebugNs=0, rendererGpuPbrGBufferDebugNs=0, rendererGpuPbrOpaqueSceneNs=0, rendererGpuPbrTransparentSceneNs=0, pbrGBufferDrawCalls=25, pbrGBufferReady=yes, pbrGBufferSize=1280x720, pbrDeferredLightingDrawCalls=1, pbrDeferredTiledLightDebugDrawCalls=0, pbrDeferredClusteredLightDebugDrawCalls=0, pbrDeferredCsmShadowBound=yes, pbrDeferredCsmShadowLayers=5, pbrDeferredCsmShadowAtlasBound=yes, pbrDeferredPointShadowAtlasBound=yes, pbrDeferredPointShadowAtlasLights=8, pbrDeferredLightBufferBound=yes, pbrDeferredLightBufferPointLights=8/16, pbrDeferredTiledLightsEnabled=no, pbrDeferredTiledLightGridBound=no, pbrDeferredTiledLightGridSize=0x0, pbrDeferredTiledLightGridTileSize=0, pbrDeferredTiledLightGridCutoff=0.000000, pbrDeferredTiledLightGridPointLights=0, pbrDeferredTiledLightGridFullIndices=0, pbrDeferredTiledLightGridIndices=0, pbrDeferredTiledLightGridCulledIndices=0, pbrDeferredTiledLightGridOccupiedTiles=0/0, pbrDeferredTiledLightGridEmptyTiles=0, pbrDeferredTiledLightGridMaxTileLights=0, pbrDeferredClusteredLightGridEnabled=yes, pbrDeferredClusteredLightGridBound=yes, pbrDeferredClusteredLightGridSize=80x45x24, pbrDeferredClusteredLightGridTileSize=16, pbrDeferredClusteredLightGridClusters=86400, pbrDeferredClusteredLightGridMaxLightsPerCluster=64, pbrDeferredClusteredLightGridMaxIndices=5529600, pbrDeferredClusteredLightGridPointLights=8, pbrDeferredClusteredLightGridIndices=8037, pbrDeferredClusteredLightGridCulledIndices=683163, pbrDeferredClusteredLightGridCompute=yes, pbrDeferredClusteredLightGridStatsReadback=yes, pbrDeferredClusteredLightGridLightIndexStats=yes
```
