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
	struct RuntimeScenePrepareConfig;

	class RuntimeContentSceneLifecycle
	{
	public:
		static void prepareScene(
			GLframework::AppRuntimeContext& context,
			GLengine::Engine& engine,
			GL_EXPERIMENTS::LegacyExperimentRunner& legacyExperiments,
			const RuntimeScenePrepareConfig& config
		);
	};
}
