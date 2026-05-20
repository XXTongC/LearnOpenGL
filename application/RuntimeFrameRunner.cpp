#include "RuntimeFrameRunner.h"

#include "../camera/cameracontrol.h"
#include "../materials/material.h"
#include "../renderer/renderer.h"
#include "RuntimeScenePreparer.h"

namespace GL_RUNTIME
{
	void RuntimeFrameRunner::run(
		GLframework::AppRuntimeContext& context,
		GL_EXPERIMENTS::LegacyExperimentRunner& legacyExperiments,
		const RuntimeFrameConfig& config,
		const RuntimeFrameCallbacks& callbacks
	)
	{
		context.cameracontrol->update();
		context.renderer->setClearColor(context.clearColor);
		RuntimeScenePreparer::updateLegacyExperiments(context, legacyExperiments);

		context.renderer->render(
			context.sceneOffScreen,
			context.camera,
			context.dirLight,
			context.spotLight,
			context.pointLights,
			context.ambientLight,
			context.frameRenderTargets.getSceneFbo()
		);

		context.postProcessPass.resolveMultisample(
			context.frameRenderTargets.getMultisample(),
			context.frameRenderTargets.getResolved()
		);

		if (context.postProcessSettings.bloomEnabled)
		{
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

		context.postProcessPass.renderScreenComposite(
			context.screenQuad,
			context.renderer->getShader(GLframework::MaterialType::ScreenMaterial),
			context.postProcessSettings,
			config.framebufferWidth,
			config.framebufferHeight
		);

		if (callbacks.renderUi)
		{
			callbacks.renderUi();
		}
	}
}
