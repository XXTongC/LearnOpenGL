#include "RuntimeFramePipeline.h"

#include "RuntimeFramePasses.h"

namespace GL_RUNTIME
{
	void RuntimeFramePipeline::render(
		GLframework::AppRuntimeContext& context,
		const RuntimeFramePipelineConfig& config
	)
	{
		if (context.framePipelineProfile.sceneColorPassEnabled)
		{
			RuntimeSceneColorPass::execute(context);
		}

		if (context.framePipelineProfile.sceneResolvePassEnabled)
		{
			RuntimeSceneResolvePass::execute(context);
		}

		if (context.framePipelineProfile.bloomPassEnabled)
		{
			RuntimeBloomPass::execute(context);
		}

		if (context.framePipelineProfile.screenCompositePassEnabled)
		{
			RuntimeScreenCompositePass::execute(context, config);
		}
	}
}
