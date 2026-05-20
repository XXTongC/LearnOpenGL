#include "RuntimeFramePipeline.h"

#include "RuntimeFramePassRegistry.h"

namespace GL_RUNTIME
{
	void RuntimeFramePipeline::render(
		GLframework::AppRuntimeContext& context,
		const RuntimeFramePipelineConfig& config
	)
	{
		const auto passPlan = RuntimeFramePassRegistry::buildPassPlan(context.framePipelineProfile);
		for (const auto* pass : passPlan)
		{
			if (pass && pass->shouldExecute(context))
			{
				pass->executePass(context, config);
			}
		}
	}
}
