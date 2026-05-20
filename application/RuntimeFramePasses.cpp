#include "RuntimeFramePasses.h"

#include "../materials/material.h"
#include "../renderer/renderer.h"

namespace GL_RUNTIME
{
	void RuntimeSceneColorPass::execute(GLframework::AppRuntimeContext& context)
	{
		context.renderer->render(
			context.sceneOffScreen,
			context.camera,
			context.dirLight,
			context.spotLight,
			context.pointLights,
			context.ambientLight,
			context.frameRenderTargets.getSceneFbo()
		);
	}

	void RuntimeSceneResolvePass::execute(GLframework::AppRuntimeContext& context)
	{
		context.postProcessPass.resolveMultisample(
			context.frameRenderTargets.getMultisample(),
			context.frameRenderTargets.getResolved()
		);
	}

	void RuntimeBloomPass::execute(GLframework::AppRuntimeContext& context)
	{
		if (!context.postProcessSettings.bloomEnabled)
		{
			return;
		}

		context.postProcessPass.extractBloomBright(
			context.bloom,
			context.frameRenderTargets.getResolved(),
			context.frameRenderTargets.getBloomBright(),
			context.postProcessSettings.bloomThreshold
		);
		context.postProcessPass.blurBloom(
			context.bloom,
			context.frameRenderTargets.getBloomBright(),
			context.frameRenderTargets.getBloomPing(),
			context.frameRenderTargets.getBloomPong(),
			context.postProcessSettings.bloomIterations
		);
	}

	void RuntimeScreenCompositePass::execute(
		GLframework::AppRuntimeContext& context,
		const RuntimeFramePipelineConfig& config
	)
	{
		context.postProcessPass.renderScreenComposite(
			context.screenQuad,
			context.renderer->getShader(GLframework::MaterialType::ScreenMaterial),
			context.postProcessSettings,
			config.framebufferWidth,
			config.framebufferHeight
		);
	}
}
