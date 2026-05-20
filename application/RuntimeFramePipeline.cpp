#include "RuntimeFramePipeline.h"

#include "RuntimeFramePasses.h"

namespace GL_RUNTIME
{
	void RuntimeFramePipeline::render(
		GLframework::AppRuntimeContext& context,
		const RuntimeFramePipelineConfig& config
	)
	{
		RuntimeSceneColorPass::execute(context);
		RuntimeSceneResolvePass::execute(context);
		RuntimeBloomPass::execute(context);
		RuntimeScreenCompositePass::execute(context, config);
	}
}
