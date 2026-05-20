#include "RendererFramePassRegistry.h"

#include "FrameRenderState.h"
#include "PBRDepthPrepass.h"
#include "PBRSceneRenderPass.h"
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
	const std::vector<std::shared_ptr<PointLight>>& pointLightsOrEmpty(const RendererFrameContext& context)
	{
		static const std::vector<std::shared_ptr<PointLight>> emptyPointLights{};
		return context.pointLights ? *context.pointLights : emptyPointLights;
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

		context.shadowRenderer->render(
			context.camera,
			context.renderQueue->getOpacityObjects(),
			context.dirLight,
			pointLightsOrEmpty(context),
			*context.shaderLibrary
		);
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
			context.camera,
			context.dirLight,
			context.spotLight,
			pointLightsOrEmpty(context),
			context.ambient,
			context.globalMaterial,
			*context.shaderLibrary,
			context.environmentTargets
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
			context.camera,
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
			context.camera,
			context.dirLight,
			context.spotLight,
			pointLightsOrEmpty(context),
			context.ambient,
			nullptr,
			*context.shaderLibrary,
			context.environmentTargets
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
			context.camera,
			context.dirLight,
			context.spotLight,
			pointLightsOrEmpty(context),
			context.ambient,
			*context.shaderLibrary,
			context.environmentTargets
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
			context.camera,
			context.dirLight,
			context.spotLight,
			pointLightsOrEmpty(context),
			context.ambient,
			nullptr,
			*context.shaderLibrary,
			context.environmentTargets
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
			context.camera,
			context.dirLight,
			context.spotLight,
			pointLightsOrEmpty(context),
			context.ambient,
			*context.shaderLibrary,
			context.environmentTargets
		);
	}
}

const std::vector<RendererFramePassDefinition>& RendererFramePassRegistry::defaultPasses()
{
	static const std::vector<RendererFramePassDefinition> passes{
		{ RendererFramePassKey::BeginFrame, "Begin Frame" },
		{ RendererFramePassKey::ShadowMaps, "Shadow Maps" },
		{ RendererFramePassKey::PBRDepthPrepass, "PBR Depth Prepass" },
		{ RendererFramePassKey::LegacyOpaqueScene, "Legacy Opaque Scene" },
		{ RendererFramePassKey::PBROpaqueScene, "PBR Opaque Scene" },
		{ RendererFramePassKey::LegacyTransparentScene, "Legacy Transparent Scene" },
		{ RendererFramePassKey::PBRTransparentScene, "PBR Transparent Scene" }
	};
	return passes;
}

const std::vector<RendererFramePassDefinition>& RendererFramePassRegistry::globalMaterialOverridePasses()
{
	static const std::vector<RendererFramePassDefinition> passes{
		{ RendererFramePassKey::BeginFrame, "Begin Frame" },
		{ RendererFramePassKey::ShadowMaps, "Shadow Maps" },
		{ RendererFramePassKey::GlobalMaterialScene, "Global Material Scene" }
	};
	return passes;
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
	case RendererFramePassKey::GlobalMaterialScene:
		renderGlobalMaterialScene(context);
		break;
	case RendererFramePassKey::PBRDepthPrepass:
		renderPBRDepthPrepass(context);
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
	default:
		break;
	}
}
