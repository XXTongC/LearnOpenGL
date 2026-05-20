#pragma once

#include "AppRuntimeContext.h"
#include "../tools/legacyExperiments/LegacyExperimentRunner.h"

namespace GL_RUNTIME
{
	struct RuntimeFrameConfig
	{
		unsigned int framebufferWidth{ 0 };
		unsigned int framebufferHeight{ 0 };
	};

	struct RuntimeFrameCallbacks
	{
		void (*renderUi)(){ nullptr };
	};

	class RuntimeFrameRunner
	{
	public:
		static void run(
			GLframework::AppRuntimeContext& context,
			GL_EXPERIMENTS::LegacyExperimentRunner& legacyExperiments,
			const RuntimeFrameConfig& config,
			const RuntimeFrameCallbacks& callbacks = {}
		);
	};
}
