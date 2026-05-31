#pragma once

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GLengine
{
	class Engine;
}

namespace GL_RUNTIME
{
	struct RuntimeScenePrepareConfig;

	class RuntimeSceneSetupPipelineLifecycle
	{
	public:
		static void prepare(
			GLframework::AppRuntimeContext& context,
			GLengine::Engine& engine,
			const RuntimeScenePrepareConfig& config
		);
	};
}
