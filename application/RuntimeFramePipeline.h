#pragma once

#include "AppRuntimeContext.h"

namespace GL_RUNTIME
{
	struct RuntimeFramePipelineConfig
	{
		unsigned int framebufferWidth{ 0 };
		unsigned int framebufferHeight{ 0 };
	};

	struct RuntimeFramePipelineStats
	{
		int plannedPassCount{ 0 };
		int executedPassCount{ 0 };
		int skippedPassCount{ 0 };
	};

	class RuntimeFramePipeline
	{
	public:
		static RuntimeFramePipelineStats render(
			GLframework::AppRuntimeContext& context,
			const RuntimeFramePipelineConfig& config
		);
	};
}
