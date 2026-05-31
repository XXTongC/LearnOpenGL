#pragma once

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GL_EXPERIMENTS
{
	class LegacyExperimentRunner;
	struct RuntimeContext;
}

namespace GL_RUNTIME
{
	struct RuntimeScenePrepareConfig;

	class RuntimeLegacyExperimentLifecycle
	{
	public:
		static GL_EXPERIMENTS::RuntimeContext makeContext(
			GLframework::AppRuntimeContext& context
		);

		static void prepare(
			GLframework::AppRuntimeContext& context,
			GL_EXPERIMENTS::LegacyExperimentRunner& legacyExperiments,
			const RuntimeScenePrepareConfig& config
		);

		static void update(
			GLframework::AppRuntimeContext& context,
			GL_EXPERIMENTS::LegacyExperimentRunner& legacyExperiments
		);
	};
}
