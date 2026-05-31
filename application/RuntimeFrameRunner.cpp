#include "RuntimeFrameRunner.h"

#include "AppRuntimeContext.h"
#include "../camera/cameracontrol.h"
#include "../engine/Engine.h"
#include "../engine/RendererSubsystem.h"
#include "../renderer/renderer.h"
#include "RuntimeFrameCallbacks.h"
#include "RuntimeLegacyExperimentLifecycle.h"
#include "RuntimeFrameRunnerTypes.h"
#include "RuntimeRendererFrameBridgeAdapter.h"

namespace GL_RUNTIME
{
	void RuntimeFrameRunner::run(
		GLframework::AppRuntimeContext& context,
		GL_EXPERIMENTS::LegacyExperimentRunner& legacyExperiments,
		const RuntimeFrameConfig& config
	)
	{
		RuntimeFrameCallbacks callbacks{};
		run(context, legacyExperiments, config, callbacks);
	}

	void RuntimeFrameRunner::run(
		GLframework::AppRuntimeContext& context,
		GL_EXPERIMENTS::LegacyExperimentRunner& legacyExperiments,
		const RuntimeFrameConfig& config,
		const RuntimeFrameCallbacks& callbacks
	)
	{
		context.cameraLights.cameracontrol->update();
		context.renderResources.renderer->setClearColor(context.renderResources.clearColor);
		RuntimeLegacyExperimentLifecycle::update(context, legacyExperiments);
		if (config.engine)
		{
			config.engine->tick(config.deltaSeconds);
		}
		if (config.engine && config.rendererSubsystem && config.rendererSubsystem->hasRendererBackend())
		{
			const GLengine::RendererFrameIntent rendererFrameIntent{
				config.framebufferWidth,
				config.framebufferHeight
			};
			config.rendererSubsystem->renderFrameBridge(
				config.engine->getContext(),
				rendererFrameIntent
			);
		}
		else
		{
			RuntimeRendererFrameBridgeAdapter::renderRuntimeFrame(context, {
				config.framebufferWidth,
				config.framebufferHeight
			});
		}

		if (callbacks.renderUi)
		{
			callbacks.renderUi();
		}
	}
}
