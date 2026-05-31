#pragma once

#include "RuntimeFrameCallbacks.h"

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GL_EXPERIMENTS
{
	class LegacyExperimentRunner;
}

namespace GL_RUNTIME
{
	struct RuntimeFrameConfig;

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
