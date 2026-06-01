#include "RuntimeFramePasses.h"

#include "AppRuntimeContext.h"
#include "RuntimeFrameRenderResourceAdapter.h"
#include "RuntimeFramePipeline.h"

namespace GL_RUNTIME
{
	void RuntimeSceneColorPass::execute(GLframework::AppRuntimeContext& context)
	{
		RuntimeFrameRenderResourceAdapter::renderSceneColor(context.renderResources, context.cameraLights);
	}

	void RuntimeSceneResolvePass::execute(GLframework::AppRuntimeContext& context)
	{
		RuntimeFrameRenderResourceAdapter::resolveSceneColor(context.renderResources);
	}

	void RuntimeBloomPass::execute(GLframework::AppRuntimeContext& context)
	{
		RuntimeFrameRenderResourceAdapter::renderBloom(
			context.renderResources,
			context.profiles.postProcessSettings()
		);
	}

	void RuntimeScreenCompositePass::execute(
		GLframework::AppRuntimeContext& context,
		const RuntimeFramePipelineConfig& config
	)
	{
		RuntimeFrameRenderResourceAdapter::renderScreenComposite(
			context.renderResources,
			context.profiles.postProcessSettings(),
			config.framebufferWidth,
			config.framebufferHeight
		);
	}
}
