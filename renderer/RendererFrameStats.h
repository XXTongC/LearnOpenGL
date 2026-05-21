#pragma once

namespace GLframework
{
	struct RendererFrameStats
	{
		int rendererPassCount{ 0 };
		int shadowCasterCount{ 0 };
		int directionalShadowLayerCount{ 0 };
		int directionalShadowDrawCalls{ 0 };
		int directionalAlphaMaskedShadowDrawCalls{ 0 };
		int pointShadowLightCount{ 0 };
		int pointShadowFaceCount{ 0 };
		int pointShadowDrawCalls{ 0 };
		int pointAlphaMaskedShadowDrawCalls{ 0 };
		bool pbrShadowAtlasReady{ false };
		int pbrShadowAtlasDirectionalLayers{ 0 };
		int pbrShadowAtlasPointLights{ 0 };
		int pbrShadowAtlasPointFaces{ 0 };
		int pbrShadowAtlasPointFacesRendered{ 0 };
		int pbrShadowAtlasDirectionalDrawCalls{ 0 };
		int pbrShadowAtlasPointDrawCalls{ 0 };
		int pbrShadowAtlasDirectionalAlphaMaskedDrawCalls{ 0 };
		int pbrShadowAtlasPointAlphaMaskedDrawCalls{ 0 };
		int pbrShadowAtlasDirectionalResolution{ 0 };
		int pbrShadowAtlasPointResolution{ 0 };
		int pbrDepthPrepassDrawCalls{ 0 };
		int pbrGBufferDrawCalls{ 0 };
		int pbrGBufferWidth{ 0 };
		int pbrGBufferHeight{ 0 };
		bool pbrGBufferReady{ false };
		int pbrDeferredLightingDrawCalls{ 0 };
		bool pbrDeferredCsmShadowBound{ false };
		int pbrDeferredCsmShadowLayers{ 0 };
		bool pbrDeferredCsmShadowAtlasBound{ false };
		bool pbrDeferredPointShadowAtlasBound{ false };
		int pbrDeferredPointShadowAtlasLights{ 0 };
		bool pbrDeferredLightBufferBound{ false };
		int pbrDeferredLightBufferPointLights{ 0 };
		int pbrDeferredLightBufferMaxPointLights{ 0 };
		bool pbrDeferredTiledLightsEnabled{ false };
		bool pbrDeferredTiledLightGridBound{ false };
		int pbrDeferredTiledLightGridTileSize{ 0 };
		float pbrDeferredTiledLightGridCutoff{ 0.0f };
		int pbrDeferredTiledLightGridColumns{ 0 };
		int pbrDeferredTiledLightGridRows{ 0 };
		int pbrDeferredTiledLightGridTileCount{ 0 };
		int pbrDeferredTiledLightGridPointLights{ 0 };
		int pbrDeferredTiledLightGridFullIndexCount{ 0 };
		int pbrDeferredTiledLightGridIndexCount{ 0 };
		int pbrDeferredTiledLightGridCulledIndexCount{ 0 };
		int pbrDeferredTiledLightGridOccupiedTiles{ 0 };
		int pbrDeferredTiledLightGridEmptyTiles{ 0 };
		int pbrDeferredTiledLightGridMaxTileLights{ 0 };
		bool pbrDeferredClusteredLightGridEnabled{ false };
		bool pbrDeferredClusteredLightGridBound{ false };
		int pbrDeferredClusteredLightGridTileSize{ 0 };
		int pbrDeferredClusteredLightGridColumns{ 0 };
		int pbrDeferredClusteredLightGridRows{ 0 };
		int pbrDeferredClusteredLightGridDepthSlices{ 0 };
		int pbrDeferredClusteredLightGridClusterCount{ 0 };
		int pbrDeferredClusteredLightGridMaxLightsPerCluster{ 0 };
		int pbrDeferredClusteredLightGridMaxIndexCount{ 0 };
		int pbrDeferredClusteredLightGridPointLights{ 0 };
		int pbrDeferredClusteredLightGridIndexCount{ 0 };
		int pbrDeferredClusteredLightGridCulledIndexCount{ 0 };
		bool pbrDeferredClusteredLightGridComputeDispatched{ false };
		bool pbrDeferredClusteredLightGridStatsReadbackEnabled{ false };
		bool pbrDeferredClusteredLightGridLightIndexStatsAvailable{ false };
		int pbrDeferredTiledLightDebugDrawCalls{ 0 };
		int pbrGBufferDebugDrawCalls{ 0 };
		int legacySceneDrawCalls{ 0 };
		int legacyTransparentDrawCalls{ 0 };
		int pbrSceneDrawCalls{ 0 };
		int pbrTransparentDrawCalls{ 0 };
		int iblDebugDrawCalls{ 0 };
	};
}
