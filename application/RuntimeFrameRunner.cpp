#include "RuntimeFrameRunner.h"

#include "../camera/cameracontrol.h"
#include "../renderer/renderer.h"
#include "RuntimeFramePipeline.h"
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
		RuntimeFramePipeline::render(context, { config.framebufferWidth, config.framebufferHeight });

		if (callbacks.renderUi)
		{
			callbacks.renderUi();
		}
	}
}
