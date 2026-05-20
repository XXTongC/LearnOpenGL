#pragma once

#include "AppRuntimeContext.h"

namespace GL_RUNTIME
{
	struct RuntimeFramePipelineConfig
	{
		unsigned int framebufferWidth{ 0 };
		unsigned int framebufferHeight{ 0 };
	};

	class RuntimeFramePipeline
	{
	public:
		static void render(
			GLframework::AppRuntimeContext& context,
			const RuntimeFramePipelineConfig& config
		);
	};
}
