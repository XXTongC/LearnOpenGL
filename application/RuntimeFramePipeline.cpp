#include "RuntimeFramePipeline.h"

#include "RuntimeFramePassRegistry.h"

namespace GL_RUNTIME
{
	RuntimeFramePipelineStats RuntimeFramePipeline::render(
		GLframework::AppRuntimeContext& context,
		const RuntimeFramePipelineConfig& config
	)
	{
		const auto passPlan = RuntimeFramePassRegistry::buildPassPlan(context.profiles.framePipelineProfile);
		RuntimeFramePipelineStats stats{};
		stats.plannedPassCount = static_cast<int>(passPlan.size());
		for (const auto* pass : passPlan)
		{
			if (!pass || !pass->shouldExecute(context))
			{
				++stats.skippedPassCount;
				continue;
			}

			if (pass)
			{
				pass->executePass(context, config);
				++stats.executedPassCount;
			}
		}
		return stats;
	}
}
