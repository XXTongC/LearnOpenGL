#include "RuntimeFramePipeline.h"

#include "RuntimeFramePassRegistry.h"

namespace GL_RUNTIME
{
	void RuntimeFramePipeline::render(
		GLframework::AppRuntimeContext& context,
		const RuntimeFramePipelineConfig& config
	)
	{
		for (const auto& pass : RuntimeFramePassRegistry::defaultPasses())
		{
			if (pass.shouldExecute(context))
			{
				pass.executePass(context, config);
			}
		}
	}
}
