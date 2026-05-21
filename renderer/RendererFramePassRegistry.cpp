#include "RendererFramePassRegistry.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <sstream>

#include "core.h"
#include "FrameRenderState.h"
#include "IBLDebugPass.h"
#include "MaterialBindingContext.h"
#include "PBRDepthPrepass.h"
#include "PBRDeferredClusteredLightDebugPass.h"
#include "PBRDeferredLightingPass.h"
#include "PBRDeferredTiledLightDebugPass.h"
#include "PBRGBufferDebugPass.h"
#include "PBRGBufferPass.h"
#include "PBRGBufferRenderTargets.h"
#include "PBRSceneRenderPass.h"
#include "PBRShadowAtlasRenderPass.h"
#include "PBRShadowAtlasRenderTargets.h"
#include "RenderQueue.h"
#include "RendererFrameContext.h"
#include "RendererFramePassProfile.h"
#include "SceneRenderPass.h"
#include "ShaderLibrary.h"
#include "ShadowRenderer.h"
#include "light/ambientLight.h"
#include "light/directionalLight.h"
#include "light/pointLight.h"
#include "light/spotLight.h"

using namespace GLframework;

namespace
{
	std::string trim(std::string value)
	{
		auto isSpace = [](unsigned char ch)
		{
			return std::isspace(ch) != 0;
		};

		value.erase(value.begin(), std::find_if(value.begin(), value.end(), [isSpace](char ch)
		{
			return !isSpace(static_cast<unsigned char>(ch));
		}));
		value.erase(std::find_if(value.rbegin(), value.rend(), [isSpace](char ch)
		{
			return !isSpace(static_cast<unsigned char>(ch));
		}).base(), value.end());
		return value;
	}

	const std::vector<std::shared_ptr<PointLight>>& pointLightsOrEmpty(const RendererFrameContext& context)
	{
		static const std::vector<std::shared_ptr<PointLight>> emptyPointLights{};
		return context.pointLights ? *context.pointLights : emptyPointLights;
	}

	const std::vector<RendererFramePassDefinition>& optionalPasses()
	{
		static const std::vector<RendererFramePassDefinition> passes{
			{ RendererFramePassKey::PBRGBuffer, "PBRGBuffer", "PBR GBuffer" },
			{ RendererFramePassKey::PBRDeferredLighting, "PBRDeferredLighting", "PBR Deferred Lighting" },
			{ RendererFramePassKey::PBRDeferredTiledLightDebug, "PBRDeferredTiledLightDebug", "PBR Deferred Tiled Light Debug" },
			{ RendererFramePassKey::PBRDeferredClusteredLightDebug, "PBRDeferredClusteredLightDebug", "PBR Deferred Clustered Light Debug" },
			{ RendererFramePassKey::PBRGBufferDebug, "PBRGBufferDebug", "PBR GBuffer Debug" },
			{ RendererFramePassKey::IBLDebug, "IBLDebug", "IBL Debug" }
		};
		return passes;
	}

	MaterialBindingContext createMaterialBindingContext(const RendererFrameContext& context)
	{
		return MaterialBindingContext{
			context.camera,
			context.dirLight,
			context.spotLight,
			context.pointLights,
			context.ambient,
			context.environmentTargets,
			context.pbrShadowAtlasTargets
		};
	}

	void beginFrame(RendererFrameContext& context)
	{
		if (context.frameRenderState)
		{
			context.frameRenderState->begin(context.fbo);
		}

		if (context.renderQueue)
		{
			context.renderQueue->build(context.scene, context.camera);
			if (context.stats)
			{
				context.stats->shadowCasterCount = static_cast<int>(context.renderQueue->getOpacityObjects().size());
			}
		}
	}

	void renderShadowMaps(RendererFrameContext& context)
	{
		if (!context.shadowRenderer || !context.renderQueue || !context.shaderLibrary)
		{
			return;
		}

		const auto stats = context.shadowRenderer->render(
			context.camera,
			context.renderQueue->getOpacityObjects(),
			context.dirLight,
			pointLightsOrEmpty(context),
			*context.shaderLibrary
		);
		if (context.stats)
		{
			context.stats->directionalShadowLayerCount = stats.directionalLayerCount;
			context.stats->directionalShadowDrawCalls = stats.directionalDrawCalls;
			context.stats->directionalAlphaMaskedShadowDrawCalls = stats.directionalAlphaMaskedDrawCalls;
			context.stats->pointShadowLightCount = stats.pointLightCount;
			context.stats->pointShadowFaceCount = stats.pointFaceCount;
			context.stats->pointShadowDrawCalls = stats.pointDrawCalls;
			context.stats->pointAlphaMaskedShadowDrawCalls = stats.pointAlphaMaskedDrawCalls;
		}
	}

	void renderPBRShadowAtlas(RendererFrameContext& context)
	{
		if (!context.pbrShadowAtlasPass || !context.pbrShadowAtlasTargets || !context.renderQueue || !context.shaderLibrary)
		{
			return;
		}

		const auto atlasStats = context.pbrShadowAtlasPass->render(
			context.camera,
			context.renderQueue->getOpacityObjects(),
			context.dirLight,
			pointLightsOrEmpty(context),
			*context.pbrShadowAtlasTargets,
			*context.shaderLibrary
		);
		if (context.stats)
		{
			context.stats->pbrShadowAtlasReady = atlasStats.ready;
			context.stats->pbrShadowAtlasDirectionalLayers = atlasStats.directionalLayerCount;
			context.stats->pbrShadowAtlasPointLights = atlasStats.pointLightCount;
			context.stats->pbrShadowAtlasPointFaces = atlasStats.pointFaceCount;
			context.stats->pbrShadowAtlasPointFacesRendered = atlasStats.pointFacesRendered;
			context.stats->pbrShadowAtlasDirectionalDrawCalls = atlasStats.directionalDrawCalls;
			context.stats->pbrShadowAtlasPointDrawCalls = atlasStats.pointDrawCalls;
			context.stats->pbrShadowAtlasDirectionalAlphaMaskedDrawCalls = atlasStats.directionalAlphaMaskedDrawCalls;
			context.stats->pbrShadowAtlasPointAlphaMaskedDrawCalls = atlasStats.pointAlphaMaskedDrawCalls;
			context.stats->pbrShadowAtlasDirectionalResolution = atlasStats.directionalResolution;
			context.stats->pbrShadowAtlasPointResolution = atlasStats.pointResolution;
		}
	}

	void renderGlobalMaterialScene(RendererFrameContext& context)
	{
		if (!context.sceneRenderPass || !context.renderQueue || !context.shaderLibrary || !context.stats)
		{
			return;
		}

		context.stats->legacySceneDrawCalls = context.sceneRenderPass->render(
			context.renderQueue->getOpacityObjects(),
			context.renderQueue->getTransparentObjects(),
			context.globalMaterial,
			*context.shaderLibrary,
			createMaterialBindingContext(context)
		);
	}

	void renderPBRDepthPrepass(RendererFrameContext& context)
	{
		if (!context.pbrDepthPrepass || !context.renderQueue || !context.shaderLibrary || !context.stats)
		{
			return;
		}

		context.stats->pbrDepthPrepassDrawCalls = context.pbrDepthPrepass->render(
			context.renderQueue->getPbrOpacityObjects(),
			createMaterialBindingContext(context),
			*context.shaderLibrary
		);
	}

	void renderPBRGBuffer(RendererFrameContext& context)
	{
		if (!context.pbrGBufferPass || !context.pbrGBufferTargets || !context.renderQueue || !context.shaderLibrary || !context.stats)
		{
			return;
		}

		const auto stats = context.pbrGBufferPass->render(
			context.renderQueue->getPbrOpacityObjects(),
			createMaterialBindingContext(context),
			*context.shaderLibrary,
			*context.pbrGBufferTargets
		);
		context.stats->pbrGBufferDrawCalls = stats.drawCalls;
		context.stats->pbrGBufferReady = stats.ready;
		context.stats->pbrGBufferWidth = stats.targetWidth;
		context.stats->pbrGBufferHeight = stats.targetHeight;
	}

	void renderPBRDeferredLighting(RendererFrameContext& context)
	{
		if (!context.pbrDeferredLightingPass || !context.pbrGBufferTargets || !context.framePassProfile || !context.shaderLibrary || !context.stats)
		{
			return;
		}

		const auto stats = context.pbrDeferredLightingPass->render(
			*context.pbrGBufferTargets,
			createMaterialBindingContext(context),
			*context.framePassProfile,
			*context.shaderLibrary
		);
		context.stats->pbrDeferredLightingDrawCalls += stats.drawCalls;
		context.stats->pbrDeferredCsmShadowBound = context.stats->pbrDeferredCsmShadowBound || stats.csmShadowBound;
		context.stats->pbrDeferredCsmShadowLayers = std::max(context.stats->pbrDeferredCsmShadowLayers, stats.csmLayerCount);
		context.stats->pbrDeferredCsmShadowAtlasBound = context.stats->pbrDeferredCsmShadowAtlasBound || stats.csmShadowAtlasBound;
		context.stats->pbrDeferredPointShadowAtlasBound = context.stats->pbrDeferredPointShadowAtlasBound || stats.pointShadowAtlasBound;
		context.stats->pbrDeferredPointShadowAtlasLights = std::max(context.stats->pbrDeferredPointShadowAtlasLights, stats.pointShadowAtlasLightCount);
		context.stats->pbrDeferredLightBufferBound = context.stats->pbrDeferredLightBufferBound || stats.lightBufferBound;
		context.stats->pbrDeferredLightBufferPointLights = std::max(context.stats->pbrDeferredLightBufferPointLights, stats.lightBufferPointLightCount);
		context.stats->pbrDeferredLightBufferMaxPointLights = std::max(context.stats->pbrDeferredLightBufferMaxPointLights, stats.lightBufferMaxPointLightCount);
		context.stats->pbrDeferredTiledLightsEnabled = context.stats->pbrDeferredTiledLightsEnabled || stats.tiledLightGridEnabled;
		context.stats->pbrDeferredTiledLightGridBound = context.stats->pbrDeferredTiledLightGridBound || stats.tiledLightGridBound;
		context.stats->pbrDeferredTiledLightGridTileSize = std::max(context.stats->pbrDeferredTiledLightGridTileSize, stats.tiledLightGridTileSize);
		context.stats->pbrDeferredTiledLightGridCutoff = std::max(context.stats->pbrDeferredTiledLightGridCutoff, stats.tiledLightGridCutoff);
		context.stats->pbrDeferredTiledLightGridColumns = std::max(context.stats->pbrDeferredTiledLightGridColumns, stats.tiledLightGridColumns);
		context.stats->pbrDeferredTiledLightGridRows = std::max(context.stats->pbrDeferredTiledLightGridRows, stats.tiledLightGridRows);
		context.stats->pbrDeferredTiledLightGridTileCount = std::max(context.stats->pbrDeferredTiledLightGridTileCount, stats.tiledLightGridTileCount);
		context.stats->pbrDeferredTiledLightGridPointLights = std::max(context.stats->pbrDeferredTiledLightGridPointLights, stats.tiledLightGridPointLightCount);
		context.stats->pbrDeferredTiledLightGridFullIndexCount = std::max(context.stats->pbrDeferredTiledLightGridFullIndexCount, stats.tiledLightGridFullIndexCount);
		context.stats->pbrDeferredTiledLightGridIndexCount = std::max(context.stats->pbrDeferredTiledLightGridIndexCount, stats.tiledLightGridIndexCount);
		context.stats->pbrDeferredTiledLightGridCulledIndexCount = std::max(context.stats->pbrDeferredTiledLightGridCulledIndexCount, stats.tiledLightGridCulledIndexCount);
		context.stats->pbrDeferredTiledLightGridOccupiedTiles = std::max(context.stats->pbrDeferredTiledLightGridOccupiedTiles, stats.tiledLightGridOccupiedTileCount);
		context.stats->pbrDeferredTiledLightGridEmptyTiles = std::max(context.stats->pbrDeferredTiledLightGridEmptyTiles, stats.tiledLightGridEmptyTileCount);
		context.stats->pbrDeferredTiledLightGridMaxTileLights = std::max(context.stats->pbrDeferredTiledLightGridMaxTileLights, stats.tiledLightGridMaxTileLightCount);
		context.stats->pbrDeferredClusteredLightGridEnabled = context.stats->pbrDeferredClusteredLightGridEnabled || stats.clusteredLightGridEnabled;
		context.stats->pbrDeferredClusteredLightGridBound = context.stats->pbrDeferredClusteredLightGridBound || stats.clusteredLightGridBound;
		context.stats->pbrDeferredClusteredLightGridTileSize = std::max(context.stats->pbrDeferredClusteredLightGridTileSize, stats.clusteredLightGridTileSize);
		context.stats->pbrDeferredClusteredLightGridColumns = std::max(context.stats->pbrDeferredClusteredLightGridColumns, stats.clusteredLightGridColumns);
		context.stats->pbrDeferredClusteredLightGridRows = std::max(context.stats->pbrDeferredClusteredLightGridRows, stats.clusteredLightGridRows);
		context.stats->pbrDeferredClusteredLightGridDepthSlices = std::max(context.stats->pbrDeferredClusteredLightGridDepthSlices, stats.clusteredLightGridDepthSlices);
		context.stats->pbrDeferredClusteredLightGridClusterCount = std::max(context.stats->pbrDeferredClusteredLightGridClusterCount, stats.clusteredLightGridClusterCount);
		context.stats->pbrDeferredClusteredLightGridMaxLightsPerCluster = std::max(context.stats->pbrDeferredClusteredLightGridMaxLightsPerCluster, stats.clusteredLightGridMaxLightsPerCluster);
		context.stats->pbrDeferredClusteredLightGridMaxIndexCount = std::max(context.stats->pbrDeferredClusteredLightGridMaxIndexCount, stats.clusteredLightGridMaxIndexCount);
		context.stats->pbrDeferredClusteredLightGridPointLights = std::max(context.stats->pbrDeferredClusteredLightGridPointLights, stats.clusteredLightGridPointLightCount);
		context.stats->pbrDeferredClusteredLightGridIndexCount = std::max(context.stats->pbrDeferredClusteredLightGridIndexCount, stats.clusteredLightGridIndexCount);
		context.stats->pbrDeferredClusteredLightGridCulledIndexCount = std::max(context.stats->pbrDeferredClusteredLightGridCulledIndexCount, stats.clusteredLightGridCulledIndexCount);
		context.stats->pbrDeferredClusteredLightGridComputeDispatched = context.stats->pbrDeferredClusteredLightGridComputeDispatched || stats.clusteredLightGridComputeDispatched;
		context.stats->pbrDeferredClusteredLightGridStatsReadbackEnabled = context.stats->pbrDeferredClusteredLightGridStatsReadbackEnabled || stats.clusteredLightGridStatsReadbackEnabled;
		context.stats->pbrDeferredClusteredLightGridLightIndexStatsAvailable = context.stats->pbrDeferredClusteredLightGridLightIndexStatsAvailable || stats.clusteredLightGridLightIndexStatsAvailable;
	}

	void accumulateTiledLightGridStats(RendererFrameContext& context, const PBRDeferredTiledLightGridStats& stats)
	{
		if (!context.stats)
		{
			return;
		}

		context.stats->pbrDeferredTiledLightGridBound = context.stats->pbrDeferredTiledLightGridBound || stats.bound;
		context.stats->pbrDeferredTiledLightGridTileSize = std::max(context.stats->pbrDeferredTiledLightGridTileSize, stats.tileSize);
		context.stats->pbrDeferredTiledLightGridCutoff = std::max(context.stats->pbrDeferredTiledLightGridCutoff, stats.lightCutoff);
		context.stats->pbrDeferredTiledLightGridColumns = std::max(context.stats->pbrDeferredTiledLightGridColumns, stats.tileColumns);
		context.stats->pbrDeferredTiledLightGridRows = std::max(context.stats->pbrDeferredTiledLightGridRows, stats.tileRows);
		context.stats->pbrDeferredTiledLightGridTileCount = std::max(context.stats->pbrDeferredTiledLightGridTileCount, stats.tileCount);
		context.stats->pbrDeferredTiledLightGridPointLights = std::max(context.stats->pbrDeferredTiledLightGridPointLights, stats.pointLightCount);
		context.stats->pbrDeferredTiledLightGridFullIndexCount = std::max(context.stats->pbrDeferredTiledLightGridFullIndexCount, stats.fullLightIndexCount);
		context.stats->pbrDeferredTiledLightGridIndexCount = std::max(context.stats->pbrDeferredTiledLightGridIndexCount, stats.lightIndexCount);
		context.stats->pbrDeferredTiledLightGridCulledIndexCount = std::max(context.stats->pbrDeferredTiledLightGridCulledIndexCount, stats.culledLightIndexCount);
		context.stats->pbrDeferredTiledLightGridOccupiedTiles = std::max(context.stats->pbrDeferredTiledLightGridOccupiedTiles, stats.occupiedTileCount);
		context.stats->pbrDeferredTiledLightGridEmptyTiles = std::max(context.stats->pbrDeferredTiledLightGridEmptyTiles, stats.emptyTileCount);
		context.stats->pbrDeferredTiledLightGridMaxTileLights = std::max(context.stats->pbrDeferredTiledLightGridMaxTileLights, stats.maxTileLightCount);
	}

	void accumulateClusteredLightGridStats(RendererFrameContext& context, const PBRDeferredClusteredLightGridStats& stats)
	{
		if (!context.stats)
		{
			return;
		}

		context.stats->pbrDeferredClusteredLightGridEnabled = context.stats->pbrDeferredClusteredLightGridEnabled || stats.enabled;
		context.stats->pbrDeferredClusteredLightGridBound = context.stats->pbrDeferredClusteredLightGridBound || stats.bound;
		context.stats->pbrDeferredClusteredLightGridTileSize = std::max(context.stats->pbrDeferredClusteredLightGridTileSize, stats.layout.tileSize);
		context.stats->pbrDeferredClusteredLightGridColumns = std::max(context.stats->pbrDeferredClusteredLightGridColumns, stats.layout.clusterColumns);
		context.stats->pbrDeferredClusteredLightGridRows = std::max(context.stats->pbrDeferredClusteredLightGridRows, stats.layout.clusterRows);
		context.stats->pbrDeferredClusteredLightGridDepthSlices = std::max(context.stats->pbrDeferredClusteredLightGridDepthSlices, stats.layout.clusterDepthSlices);
		context.stats->pbrDeferredClusteredLightGridClusterCount = std::max(context.stats->pbrDeferredClusteredLightGridClusterCount, stats.layout.clusterCount);
		context.stats->pbrDeferredClusteredLightGridMaxLightsPerCluster = std::max(context.stats->pbrDeferredClusteredLightGridMaxLightsPerCluster, stats.layout.maxLightsPerCluster);
		context.stats->pbrDeferredClusteredLightGridMaxIndexCount = std::max(context.stats->pbrDeferredClusteredLightGridMaxIndexCount, stats.layout.maxLightIndexCount);
		context.stats->pbrDeferredClusteredLightGridPointLights = std::max(context.stats->pbrDeferredClusteredLightGridPointLights, stats.pointLightCount);
		context.stats->pbrDeferredClusteredLightGridIndexCount = std::max(context.stats->pbrDeferredClusteredLightGridIndexCount, stats.lightIndexCount);
		context.stats->pbrDeferredClusteredLightGridCulledIndexCount = std::max(context.stats->pbrDeferredClusteredLightGridCulledIndexCount, stats.culledLightIndexCount);
		context.stats->pbrDeferredClusteredLightGridComputeDispatched = context.stats->pbrDeferredClusteredLightGridComputeDispatched || stats.computeDispatched;
		context.stats->pbrDeferredClusteredLightGridStatsReadbackEnabled = context.stats->pbrDeferredClusteredLightGridStatsReadbackEnabled || stats.statsReadbackEnabled;
		context.stats->pbrDeferredClusteredLightGridLightIndexStatsAvailable = context.stats->pbrDeferredClusteredLightGridLightIndexStatsAvailable || stats.lightIndexStatsAvailable;
	}

	void renderPBRDeferredTiledLightDebug(RendererFrameContext& context)
	{
		if (!context.pbrDeferredTiledLightDebugPass || !context.pbrGBufferTargets || !context.framePassProfile || !context.shaderLibrary || !context.stats)
		{
			return;
		}

		const auto stats = context.pbrDeferredTiledLightDebugPass->render(
			createMaterialBindingContext(context),
			*context.framePassProfile,
			*context.shaderLibrary,
			context.pbrGBufferTargets->getWidth(),
			context.pbrGBufferTargets->getHeight()
		);
		context.stats->pbrDeferredTiledLightDebugDrawCalls += stats.drawCalls;
		accumulateTiledLightGridStats(context, stats.gridStats);
	}

	void renderPBRDeferredClusteredLightDebug(RendererFrameContext& context)
	{
		if (!context.pbrDeferredClusteredLightDebugPass || !context.pbrGBufferTargets || !context.framePassProfile || !context.shaderLibrary || !context.stats)
		{
			return;
		}

		const auto stats = context.pbrDeferredClusteredLightDebugPass->render(
			createMaterialBindingContext(context),
			*context.framePassProfile,
			*context.shaderLibrary,
			context.pbrGBufferTargets->getWidth(),
			context.pbrGBufferTargets->getHeight()
		);
		context.stats->pbrDeferredClusteredLightDebugDrawCalls += stats.drawCalls;
		accumulateClusteredLightGridStats(context, stats.gridStats);
	}

	void renderPBRGBufferDebug(RendererFrameContext& context)
	{
		if (!context.pbrGBufferDebugPass || !context.pbrGBufferTargets || !context.framePassProfile || !context.shaderLibrary || !context.stats)
		{
			return;
		}

		context.stats->pbrGBufferDebugDrawCalls += context.pbrGBufferDebugPass->render(
			*context.pbrGBufferTargets,
			*context.framePassProfile,
			*context.shaderLibrary
		);
	}

	void renderLegacyOpaqueScene(RendererFrameContext& context)
	{
		if (!context.sceneRenderPass || !context.renderQueue || !context.shaderLibrary || !context.stats)
		{
			return;
		}

		context.stats->legacySceneDrawCalls += context.sceneRenderPass->render(
			context.renderQueue->getLegacyOpacityObjects(),
			{},
			nullptr,
			*context.shaderLibrary,
			createMaterialBindingContext(context)
		);
	}

	void renderPBROpaqueScene(RendererFrameContext& context)
	{
		if (!context.pbrSceneRenderPass || !context.renderQueue || !context.shaderLibrary || !context.stats)
		{
			return;
		}

		context.stats->pbrSceneDrawCalls += context.pbrSceneRenderPass->render(
			context.renderQueue->getPbrOpacityObjects(),
			{},
			*context.shaderLibrary,
			createMaterialBindingContext(context)
		);
	}

	void renderLegacyTransparentScene(RendererFrameContext& context)
	{
		if (!context.sceneRenderPass || !context.renderQueue || !context.shaderLibrary || !context.stats)
		{
			return;
		}

		const int drawCalls = context.sceneRenderPass->render(
			{},
			context.renderQueue->getLegacyTransparentObjects(),
			nullptr,
			*context.shaderLibrary,
			createMaterialBindingContext(context)
		);
		context.stats->legacySceneDrawCalls += drawCalls;
		context.stats->legacyTransparentDrawCalls += drawCalls;
	}

	void renderPBRTransparentScene(RendererFrameContext& context)
	{
		if (!context.pbrSceneRenderPass || !context.renderQueue || !context.shaderLibrary || !context.stats)
		{
			return;
		}

		const int drawCalls = context.pbrSceneRenderPass->render(
			{},
			context.renderQueue->getPbrTransparentObjects(),
			*context.shaderLibrary,
			createMaterialBindingContext(context)
		);
		context.stats->pbrSceneDrawCalls += drawCalls;
		context.stats->pbrTransparentDrawCalls += drawCalls;
	}

	void renderIBLDebug(RendererFrameContext& context)
	{
		if (!context.iblDebugPass || !context.environmentTargets || !context.framePassProfile || !context.shaderLibrary || !context.stats)
		{
			return;
		}

		context.stats->iblDebugDrawCalls += context.iblDebugPass->render(
			*context.environmentTargets,
			*context.framePassProfile,
			*context.shaderLibrary
		);
	}

	bool containsPassKey(
		const std::vector<const RendererFramePassDefinition*>& passes,
		RendererFramePassKey key
	)
	{
		return std::any_of(passes.begin(), passes.end(), [key](const RendererFramePassDefinition* pass)
		{
			return pass && pass->key == key;
		});
	}

	bool isGpuTimingEnabled(const RendererFrameContext& context)
	{
		return context.stats && context.framePassProfile && context.framePassProfile->rendererGpuTimingEnabled;
	}

	void accumulateGpuPassTime(
		RendererFrameStats& stats,
		RendererFramePassKey key,
		std::uint64_t elapsedNs
	)
	{
		stats.rendererGpuTimingEnabled = true;
		stats.rendererGpuTimingAvailable = true;
		++stats.rendererGpuTimedPassCount;
		stats.rendererGpuFrameTimeNs += elapsedNs;

		switch (key)
		{
		case RendererFramePassKey::BeginFrame:
			stats.rendererGpuBeginFrameTimeNs += elapsedNs;
			break;
		case RendererFramePassKey::ShadowMaps:
			stats.rendererGpuShadowMapsTimeNs += elapsedNs;
			break;
		case RendererFramePassKey::PBRShadowAtlas:
			stats.rendererGpuPbrShadowAtlasTimeNs += elapsedNs;
			break;
		case RendererFramePassKey::PBRDepthPrepass:
			stats.rendererGpuPbrDepthPrepassTimeNs += elapsedNs;
			break;
		case RendererFramePassKey::PBRGBuffer:
			stats.rendererGpuPbrGBufferTimeNs += elapsedNs;
			break;
		case RendererFramePassKey::PBRDeferredLighting:
			stats.rendererGpuPbrDeferredLightingTimeNs += elapsedNs;
			break;
		case RendererFramePassKey::PBRDeferredTiledLightDebug:
			stats.rendererGpuPbrDeferredTiledLightDebugTimeNs += elapsedNs;
			break;
		case RendererFramePassKey::PBRDeferredClusteredLightDebug:
			stats.rendererGpuPbrDeferredClusteredLightDebugTimeNs += elapsedNs;
			break;
		case RendererFramePassKey::PBRGBufferDebug:
			stats.rendererGpuPbrGBufferDebugTimeNs += elapsedNs;
			break;
		case RendererFramePassKey::PBROpaqueScene:
			stats.rendererGpuPbrOpaqueSceneTimeNs += elapsedNs;
			break;
		case RendererFramePassKey::PBRTransparentScene:
			stats.rendererGpuPbrTransparentSceneTimeNs += elapsedNs;
			break;
		default:
			break;
		}
	}

	void executePassBody(const RendererFramePassDefinition& pass, RendererFrameContext& context)
	{
		switch (pass.key)
		{
		case RendererFramePassKey::BeginFrame:
			beginFrame(context);
			break;
		case RendererFramePassKey::ShadowMaps:
			renderShadowMaps(context);
			break;
		case RendererFramePassKey::PBRShadowAtlas:
			renderPBRShadowAtlas(context);
			break;
		case RendererFramePassKey::GlobalMaterialScene:
			renderGlobalMaterialScene(context);
			break;
		case RendererFramePassKey::PBRDepthPrepass:
			renderPBRDepthPrepass(context);
			break;
		case RendererFramePassKey::PBRGBuffer:
			renderPBRGBuffer(context);
			break;
		case RendererFramePassKey::PBRDeferredLighting:
			renderPBRDeferredLighting(context);
			break;
		case RendererFramePassKey::PBRDeferredTiledLightDebug:
			renderPBRDeferredTiledLightDebug(context);
			break;
		case RendererFramePassKey::PBRDeferredClusteredLightDebug:
			renderPBRDeferredClusteredLightDebug(context);
			break;
		case RendererFramePassKey::PBRGBufferDebug:
			renderPBRGBufferDebug(context);
			break;
		case RendererFramePassKey::LegacyOpaqueScene:
			renderLegacyOpaqueScene(context);
			break;
		case RendererFramePassKey::PBROpaqueScene:
			renderPBROpaqueScene(context);
			break;
		case RendererFramePassKey::LegacyTransparentScene:
			renderLegacyTransparentScene(context);
			break;
		case RendererFramePassKey::PBRTransparentScene:
			renderPBRTransparentScene(context);
			break;
		case RendererFramePassKey::IBLDebug:
			renderIBLDebug(context);
			break;
		default:
			break;
		}
	}

	void executeTimedPass(const RendererFramePassDefinition& pass, RendererFrameContext& context)
	{
		if (!isGpuTimingEnabled(context))
		{
			executePassBody(pass, context);
			return;
		}

		context.stats->rendererGpuTimingEnabled = true;

		GLuint queryId = 0;
		glGenQueries(1, &queryId);
		if (queryId == 0)
		{
			executePassBody(pass, context);
			return;
		}

		glBeginQuery(GL_TIME_ELAPSED, queryId);
		executePassBody(pass, context);
		glEndQuery(GL_TIME_ELAPSED);

		GLuint64 elapsedNs = 0;
		glGetQueryObjectui64v(queryId, GL_QUERY_RESULT, &elapsedNs);
		glDeleteQueries(1, &queryId);
		accumulateGpuPassTime(*context.stats, pass.key, static_cast<std::uint64_t>(elapsedNs));
	}
}

const std::vector<RendererFramePassDefinition>& RendererFramePassRegistry::defaultPasses()
{
	static const std::vector<RendererFramePassDefinition> passes{
		{ RendererFramePassKey::BeginFrame, "BeginFrame", "Begin Frame" },
		{ RendererFramePassKey::ShadowMaps, "ShadowMaps", "Shadow Maps" },
		{ RendererFramePassKey::PBRShadowAtlas, "PBRShadowAtlas", "PBR Shadow Atlas" },
		{ RendererFramePassKey::PBRDepthPrepass, "PBRDepthPrepass", "PBR Depth Prepass" },
		{ RendererFramePassKey::LegacyOpaqueScene, "LegacyOpaqueScene", "Legacy Opaque Scene" },
		{ RendererFramePassKey::PBROpaqueScene, "PBROpaqueScene", "PBR Opaque Scene" },
		{ RendererFramePassKey::LegacyTransparentScene, "LegacyTransparentScene", "Legacy Transparent Scene" },
		{ RendererFramePassKey::PBRTransparentScene, "PBRTransparentScene", "PBR Transparent Scene" }
	};
	return passes;
}

const std::vector<RendererFramePassDefinition>& RendererFramePassRegistry::globalMaterialOverridePasses()
{
	static const std::vector<RendererFramePassDefinition> passes{
		{ RendererFramePassKey::BeginFrame, "BeginFrame", "Begin Frame" },
		{ RendererFramePassKey::ShadowMaps, "ShadowMaps", "Shadow Maps" },
		{ RendererFramePassKey::GlobalMaterialScene, "GlobalMaterialScene", "Global Material Scene" }
	};
	return passes;
}

const char* RendererFramePassRegistry::defaultPassOrder()
{
	return "BeginFrame,ShadowMaps,PBRShadowAtlas,PBRDepthPrepass,LegacyOpaqueScene,PBROpaqueScene,LegacyTransparentScene,PBRTransparentScene";
}

const char* RendererFramePassRegistry::globalMaterialOverridePassOrder()
{
	return "BeginFrame,ShadowMaps,GlobalMaterialScene";
}

const RendererFramePassDefinition* RendererFramePassRegistry::findPassByKey(const std::string& key)
{
	const auto normalizedKey = trim(key);
	if (normalizedKey.empty())
	{
		return nullptr;
	}

	for (const auto& pass : defaultPasses())
	{
		if (normalizedKey == pass.keyName || normalizedKey == pass.debugName)
		{
			return &pass;
		}
	}

	for (const auto& pass : globalMaterialOverridePasses())
	{
		if (normalizedKey == pass.keyName || normalizedKey == pass.debugName)
		{
			return &pass;
		}
	}

	for (const auto& pass : optionalPasses())
	{
		if (normalizedKey == pass.keyName || normalizedKey == pass.debugName)
		{
			return &pass;
		}
	}

	return nullptr;
}

std::vector<const RendererFramePassDefinition*> RendererFramePassRegistry::buildPassPlan(
	const std::string& passOrder
)
{
	std::vector<const RendererFramePassDefinition*> plan{};
	std::stringstream stream{ passOrder };
	std::string token{};
	while (std::getline(stream, token, ','))
	{
		const auto* pass = findPassByKey(token);
		if (pass && !containsPassKey(plan, pass->key))
		{
			plan.push_back(pass);
		}
	}

	if (!plan.empty())
	{
		return plan;
	}

	for (const auto& pass : defaultPasses())
	{
		plan.push_back(&pass);
	}
	return plan;
}

void RendererFramePassRegistry::executePass(const RendererFramePassDefinition& pass, RendererFrameContext& context)
{
	if (context.stats)
	{
		++context.stats->rendererPassCount;
	}

	executeTimedPass(pass, context);
}
