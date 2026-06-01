#include "RuntimePBRRendererStatsVerification.h"

#include <iostream>
#include <string>

#include "../renderer/RendererFrameStats.h"
#include "../tools/Logger/LogManager.h"
#include "AppRuntimeContext.h"
#include "RuntimePBRStatsResourceAdapter.h"

namespace
{
	void reportLine(const std::string& message)
	{
		std::cout << message << std::endl;
		LogInfo(message);
	}
}

namespace GL_RUNTIME
{
	void RuntimePBRRendererStatsVerification::reportRenderedFrame(GLframework::AppRuntimeContext& context)
	{
		const auto* rendererStats = RuntimePBRStatsResourceAdapter::lastRendererFrameStats(context.renderResources);
		if (!rendererStats)
		{
			return;
		}

		const auto& stats = *rendererStats;
		std::string statsLine =
			"PBR verification renderer stats: rendererPasses=" + std::to_string(stats.rendererPassCount)
			+ ", shadowCasters=" + std::to_string(stats.shadowCasterCount)
			+ ", directionalShadowLayers=" + std::to_string(stats.directionalShadowLayerCount)
			+ ", directionalShadowDrawCalls=" + std::to_string(stats.directionalShadowDrawCalls)
			+ ", directionalAlphaMaskedShadowDrawCalls=" + std::to_string(stats.directionalAlphaMaskedShadowDrawCalls)
			+ ", pointShadowLights=" + std::to_string(stats.pointShadowLightCount)
			+ ", pointShadowFaces=" + std::to_string(stats.pointShadowFaceCount)
			+ ", pointShadowDrawCalls=" + std::to_string(stats.pointShadowDrawCalls)
			+ ", pointAlphaMaskedShadowDrawCalls=" + std::to_string(stats.pointAlphaMaskedShadowDrawCalls)
			+ ", pbrShadowAtlasReady=" + (stats.pbrShadowAtlasReady ? std::string{ "yes" } : std::string{ "no" })
			+ ", pbrShadowAtlasDirectionalLayers=" + std::to_string(stats.pbrShadowAtlasDirectionalLayers)
			+ ", pbrShadowAtlasPointFaces=" + std::to_string(stats.pbrShadowAtlasPointFaces)
			+ ", pbrShadowAtlasPointFacesRendered=" + std::to_string(stats.pbrShadowAtlasPointFacesRendered)
			+ ", pbrShadowAtlasDirectionalDrawCalls=" + std::to_string(stats.pbrShadowAtlasDirectionalDrawCalls)
			+ ", pbrShadowAtlasPointDrawCalls=" + std::to_string(stats.pbrShadowAtlasPointDrawCalls)
			+ ", pbrShadowAtlasDirectionalAlphaMaskedDrawCalls=" + std::to_string(stats.pbrShadowAtlasDirectionalAlphaMaskedDrawCalls)
			+ ", pbrShadowAtlasPointAlphaMaskedDrawCalls=" + std::to_string(stats.pbrShadowAtlasPointAlphaMaskedDrawCalls)
			+ ", pbrDepthPrepassDrawCalls=" + std::to_string(stats.pbrDepthPrepassDrawCalls)
			+ ", legacyDrawCalls=" + std::to_string(stats.legacySceneDrawCalls)
			+ ", legacyTransparentDrawCalls=" + std::to_string(stats.legacyTransparentDrawCalls)
			+ ", pbrDrawCalls=" + std::to_string(stats.pbrSceneDrawCalls)
			+ ", pbrTransparentDrawCalls=" + std::to_string(stats.pbrTransparentDrawCalls);
		if (stats.rendererGpuTimingEnabled)
		{
			statsLine += ", rendererGpuTimingEnabled=yes";
			statsLine += ", rendererGpuTimingAvailable=";
			statsLine += (stats.rendererGpuTimingAvailable ? "yes" : "no");
			statsLine += ", rendererGpuTimingDeferredReadback=";
			statsLine += (stats.rendererGpuTimingDeferredReadback ? "yes" : "no");
			statsLine += ", rendererGpuTimedPasses=" + std::to_string(stats.rendererGpuTimedPassCount);
			statsLine += ", rendererGpuTimingPendingQueries=" + std::to_string(stats.rendererGpuTimingPendingQueries);
			statsLine += ", rendererGpuFrameNs=" + std::to_string(stats.rendererGpuFrameTimeNs);
			statsLine += ", rendererGpuBeginFrameNs=" + std::to_string(stats.rendererGpuBeginFrameTimeNs);
			statsLine += ", rendererGpuShadowMapsNs=" + std::to_string(stats.rendererGpuShadowMapsTimeNs);
			statsLine += ", rendererGpuPbrShadowAtlasNs=" + std::to_string(stats.rendererGpuPbrShadowAtlasTimeNs);
			statsLine += ", rendererGpuPbrDepthPrepassNs=" + std::to_string(stats.rendererGpuPbrDepthPrepassTimeNs);
			statsLine += ", rendererGpuPbrGBufferNs=" + std::to_string(stats.rendererGpuPbrGBufferTimeNs);
			statsLine += ", rendererGpuPbrDeferredLightingNs=" + std::to_string(stats.rendererGpuPbrDeferredLightingTimeNs);
			statsLine += ", rendererGpuPbrDeferredTiledLightDebugNs=" + std::to_string(stats.rendererGpuPbrDeferredTiledLightDebugTimeNs);
			statsLine += ", rendererGpuPbrDeferredClusteredLightDebugNs=" + std::to_string(stats.rendererGpuPbrDeferredClusteredLightDebugTimeNs);
			statsLine += ", rendererGpuPbrGBufferDebugNs=" + std::to_string(stats.rendererGpuPbrGBufferDebugTimeNs);
			statsLine += ", rendererGpuPbrOpaqueSceneNs=" + std::to_string(stats.rendererGpuPbrOpaqueSceneTimeNs);
			statsLine += ", rendererGpuPbrTransparentSceneNs=" + std::to_string(stats.rendererGpuPbrTransparentSceneTimeNs);
		}
		if (stats.iblDebugDrawCalls > 0)
		{
			statsLine += ", iblDebugDrawCalls=" + std::to_string(stats.iblDebugDrawCalls);
		}
		if (stats.pbrGBufferReady || stats.pbrGBufferDrawCalls > 0)
		{
			statsLine += ", pbrGBufferDrawCalls=" + std::to_string(stats.pbrGBufferDrawCalls);
			statsLine += ", pbrGBufferReady=";
			statsLine += (stats.pbrGBufferReady ? "yes" : "no");
			statsLine += ", pbrGBufferSize=" + std::to_string(stats.pbrGBufferWidth)
				+ "x" + std::to_string(stats.pbrGBufferHeight);
		}
		if (stats.pbrDeferredLightingDrawCalls > 0 || stats.pbrDeferredTiledLightDebugDrawCalls > 0 || stats.pbrDeferredClusteredLightDebugDrawCalls > 0)
		{
			statsLine += ", pbrDeferredLightingDrawCalls=" + std::to_string(stats.pbrDeferredLightingDrawCalls);
			statsLine += ", pbrDeferredTiledLightDebugDrawCalls=" + std::to_string(stats.pbrDeferredTiledLightDebugDrawCalls);
			statsLine += ", pbrDeferredClusteredLightDebugDrawCalls=" + std::to_string(stats.pbrDeferredClusteredLightDebugDrawCalls);
			statsLine += ", pbrDeferredCsmShadowBound=";
			statsLine += (stats.pbrDeferredCsmShadowBound ? "yes" : "no");
			statsLine += ", pbrDeferredCsmShadowLayers=" + std::to_string(stats.pbrDeferredCsmShadowLayers);
			statsLine += ", pbrDeferredCsmShadowAtlasBound=";
			statsLine += (stats.pbrDeferredCsmShadowAtlasBound ? "yes" : "no");
			statsLine += ", pbrDeferredPointShadowAtlasBound=";
			statsLine += (stats.pbrDeferredPointShadowAtlasBound ? "yes" : "no");
			statsLine += ", pbrDeferredPointShadowAtlasLights=" + std::to_string(stats.pbrDeferredPointShadowAtlasLights);
			statsLine += ", pbrDeferredLightBufferBound=";
			statsLine += (stats.pbrDeferredLightBufferBound ? "yes" : "no");
			statsLine += ", pbrDeferredLightBufferPointLights=" + std::to_string(stats.pbrDeferredLightBufferPointLights);
			statsLine += "/" + std::to_string(stats.pbrDeferredLightBufferMaxPointLights);
			statsLine += ", pbrDeferredTiledLightsEnabled=";
			statsLine += (stats.pbrDeferredTiledLightsEnabled ? "yes" : "no");
			statsLine += ", pbrDeferredTiledLightGridBound=";
			statsLine += (stats.pbrDeferredTiledLightGridBound ? "yes" : "no");
			statsLine += ", pbrDeferredTiledLightGridSize=" + std::to_string(stats.pbrDeferredTiledLightGridColumns)
				+ "x" + std::to_string(stats.pbrDeferredTiledLightGridRows);
			statsLine += ", pbrDeferredTiledLightGridTileSize=" + std::to_string(stats.pbrDeferredTiledLightGridTileSize);
			statsLine += ", pbrDeferredTiledLightGridCutoff=" + std::to_string(stats.pbrDeferredTiledLightGridCutoff);
			statsLine += ", pbrDeferredTiledLightGridPointLights=" + std::to_string(stats.pbrDeferredTiledLightGridPointLights);
			statsLine += ", pbrDeferredTiledLightGridFullIndices=" + std::to_string(stats.pbrDeferredTiledLightGridFullIndexCount);
			statsLine += ", pbrDeferredTiledLightGridIndices=" + std::to_string(stats.pbrDeferredTiledLightGridIndexCount);
			statsLine += ", pbrDeferredTiledLightGridCulledIndices=" + std::to_string(stats.pbrDeferredTiledLightGridCulledIndexCount);
			statsLine += ", pbrDeferredTiledLightGridOccupiedTiles=" + std::to_string(stats.pbrDeferredTiledLightGridOccupiedTiles);
			statsLine += "/" + std::to_string(stats.pbrDeferredTiledLightGridTileCount);
			statsLine += ", pbrDeferredTiledLightGridEmptyTiles=" + std::to_string(stats.pbrDeferredTiledLightGridEmptyTiles);
			statsLine += ", pbrDeferredTiledLightGridMaxTileLights=" + std::to_string(stats.pbrDeferredTiledLightGridMaxTileLights);
			if (stats.pbrDeferredClusteredLightGridEnabled || stats.pbrDeferredClusteredLightGridClusterCount > 0)
			{
				statsLine += ", pbrDeferredClusteredLightGridEnabled=";
				statsLine += (stats.pbrDeferredClusteredLightGridEnabled ? "yes" : "no");
				statsLine += ", pbrDeferredClusteredLightGridBound=";
				statsLine += (stats.pbrDeferredClusteredLightGridBound ? "yes" : "no");
				statsLine += ", pbrDeferredClusteredLightGridSize=" + std::to_string(stats.pbrDeferredClusteredLightGridColumns)
					+ "x" + std::to_string(stats.pbrDeferredClusteredLightGridRows)
					+ "x" + std::to_string(stats.pbrDeferredClusteredLightGridDepthSlices);
				statsLine += ", pbrDeferredClusteredLightGridTileSize=" + std::to_string(stats.pbrDeferredClusteredLightGridTileSize);
				statsLine += ", pbrDeferredClusteredLightGridClusters=" + std::to_string(stats.pbrDeferredClusteredLightGridClusterCount);
				statsLine += ", pbrDeferredClusteredLightGridMaxLightsPerCluster=" + std::to_string(stats.pbrDeferredClusteredLightGridMaxLightsPerCluster);
				statsLine += ", pbrDeferredClusteredLightGridMaxIndices=" + std::to_string(stats.pbrDeferredClusteredLightGridMaxIndexCount);
				statsLine += ", pbrDeferredClusteredLightGridPointLights=" + std::to_string(stats.pbrDeferredClusteredLightGridPointLights);
				statsLine += ", pbrDeferredClusteredLightGridIndices=" + std::to_string(stats.pbrDeferredClusteredLightGridIndexCount);
				statsLine += ", pbrDeferredClusteredLightGridCulledIndices=" + std::to_string(stats.pbrDeferredClusteredLightGridCulledIndexCount);
				statsLine += ", pbrDeferredClusteredLightGridCompute=";
				statsLine += (stats.pbrDeferredClusteredLightGridComputeDispatched ? "yes" : "no");
				statsLine += ", pbrDeferredClusteredLightGridStatsReadback=";
				statsLine += (stats.pbrDeferredClusteredLightGridStatsReadbackEnabled ? "yes" : "no");
				statsLine += ", pbrDeferredClusteredLightGridLightIndexStats=";
				statsLine += (stats.pbrDeferredClusteredLightGridLightIndexStatsAvailable ? "yes" : "no");
			}
		}
		if (stats.pbrGBufferDebugDrawCalls > 0)
		{
			statsLine += ", pbrGBufferDebugDrawCalls=" + std::to_string(stats.pbrGBufferDebugDrawCalls);
		}
		reportLine(statsLine);
	}
}
