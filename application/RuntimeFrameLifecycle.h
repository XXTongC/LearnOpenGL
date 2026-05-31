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

namespace GLengine
{
	class Engine;
	class RendererSubsystem;
}

namespace GL_RUNTIME
{
	struct RuntimeFrameLifecycleConfig;
	struct RuntimeFrameLifecycleState;

	class RuntimeFrameLifecycle
	{
	public:
		static void reset(RuntimeFrameLifecycleState& state);

		static bool shouldContinue(
			const RuntimeFrameLifecycleConfig& config,
			const RuntimeFrameLifecycleState& state
		);

		static void runFrame(
			GLframework::AppRuntimeContext& context,
			GLengine::Engine& engine,
			GLengine::RendererSubsystem* rendererSubsystem,
			GL_EXPERIMENTS::LegacyExperimentRunner& legacyExperiments,
			RuntimeFrameLifecycleState& state,
			const RuntimeFrameLifecycleConfig& config,
			unsigned int framebufferWidth,
			unsigned int framebufferHeight,
			const RuntimeFrameCallbacks& callbacks = {}
		);
	};
}
