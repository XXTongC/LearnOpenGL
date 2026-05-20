#include "RendererFramePassRegistry.h"

#include <algorithm>
#include <cctype>
#include <sstream>

#include "FrameRenderState.h"
#include "IBLDebugPass.h"
#include "MaterialBindingContext.h"
#include "PBRDepthPrepass.h"
#include "PBRDeferredLightingPass.h"
#include "PBRGBufferDebugPass.h"
#include "PBRGBufferPass.h"
#include "PBRSceneRenderPass.h"
#include "PBRShadowAtlasRenderPass.h"
#include "PBRShadowAtlasRenderTargets.h"
#include "RenderQueue.h"
#include "RendererFrameContext.h"
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
			context.stats->pointShadowLightCount = stats.pointLightCount;
			context.stats->pointShadowFaceCount = stats.pointFaceCount;
			context.stats->pointShadowDrawCalls = stats.pointDrawCalls;
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

		context.stats->legacySceneDrawCalls += context.sceneRenderPass->render(
			{},
			context.renderQueue->getLegacyTransparentObjects(),
			nullptr,
			*context.shaderLibrary,
			createMaterialBindingContext(context)
		);
	}

	void renderPBRTransparentScene(RendererFrameContext& context)
	{
		if (!context.pbrSceneRenderPass || !context.renderQueue || !context.shaderLibrary || !context.stats)
		{
			return;
		}

		context.stats->pbrSceneDrawCalls += context.pbrSceneRenderPass->render(
			{},
			context.renderQueue->getPbrTransparentObjects(),
			*context.shaderLibrary,
			createMaterialBindingContext(context)
		);
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
