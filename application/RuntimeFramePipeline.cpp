#include "RuntimeFramePipeline.h"

#include "AppRuntimeContext.h"
#include "RuntimeFramePassRegistry.h"

namespace GL_RUNTIME
{
	RuntimeFramePipelineStats RuntimeFramePipeline::render(
		GLframework::AppRuntimeContext& context,
		const RuntimeFramePipelineConfig& config
	)
	{
		const auto& profile = context.profiles.framePipelineProfile();
		const auto passPlan = RuntimeFramePassRegistry::buildPassPlan(profile);
		RuntimeFramePipelineStats stats{};
		stats.plannedPassCount = static_cast<int>(passPlan.size());
		for (const auto* pass : passPlan)
		{
			if (!pass || !pass->shouldExecute(profile))
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
