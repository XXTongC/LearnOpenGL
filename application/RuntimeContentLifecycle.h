#pragma once

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GLengine
{
	class Engine;
}

namespace GL_EXPERIMENTS
{
	class LegacyExperimentRunner;
}

namespace GL_RUNTIME
{
	struct RuntimeContentLifecycleConfig;
	struct RuntimeEngineLifecycleState;

	class RuntimeContentLifecycle
	{
	public:
		static bool prepare(
			GLframework::AppRuntimeContext& context,
			GLengine::Engine& engine,
			RuntimeEngineLifecycleState& engineLifecycle,
			GL_EXPERIMENTS::LegacyExperimentRunner& legacyExperiments,
			const RuntimeContentLifecycleConfig& config
		);
	};
}
