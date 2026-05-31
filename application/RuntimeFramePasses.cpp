#include "RuntimeFramePasses.h"

#include "AppRuntimeContext.h"
#include "RuntimeFramePipeline.h"
#include "../materials/material.h"
#include "../renderer/FrameRenderTargets.h"
#include "../renderer/PostProcessPass.h"
#include "../renderer/renderer.h"

namespace GL_RUNTIME
{
	void RuntimeSceneColorPass::execute(GLframework::AppRuntimeContext& context)
	{
		context.renderResources.renderer->render(
			context.renderResources.sceneOffScreen,
			context.cameraLights.camera,
			context.cameraLights.dirLight,
			context.cameraLights.spotLight,
			context.cameraLights.pointLights,
			context.cameraLights.ambientLight,
			context.renderResources.frameRenderTargets().getSceneFbo()
		);
	}

	void RuntimeSceneResolvePass::execute(GLframework::AppRuntimeContext& context)
	{
		context.renderResources.postProcessPass().resolveMultisample(
			context.renderResources.frameRenderTargets().getMultisample(),
			context.renderResources.frameRenderTargets().getResolved()
		);
	}

	void RuntimeBloomPass::execute(GLframework::AppRuntimeContext& context)
	{
		if (!context.profiles.postProcessSettings.bloomEnabled)
		{
			return;
		}

		context.renderResources.postProcessPass().extractBloomBright(
			context.renderResources.bloom,
			context.renderResources.frameRenderTargets().getResolved(),
			context.renderResources.frameRenderTargets().getBloomBright(),
			context.profiles.postProcessSettings.bloomThreshold
		);
		context.renderResources.postProcessPass().blurBloom(
			context.renderResources.bloom,
			context.renderResources.frameRenderTargets().getBloomBright(),
			context.renderResources.frameRenderTargets().getBloomPing(),
			context.renderResources.frameRenderTargets().getBloomPong(),
			context.profiles.postProcessSettings.bloomIterations
		);
	}

	void RuntimeScreenCompositePass::execute(
		GLframework::AppRuntimeContext& context,
		const RuntimeFramePipelineConfig& config
	)
	{
		context.renderResources.postProcessPass().renderScreenComposite(
			context.renderResources.screenQuad,
			context.renderResources.renderer->getShader(GLframework::MaterialType::ScreenMaterial),
			context.profiles.postProcessSettings,
			config.framebufferWidth,
			config.framebufferHeight
		);
	}
}
