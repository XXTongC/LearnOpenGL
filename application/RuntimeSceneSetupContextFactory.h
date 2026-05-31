#pragma once

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GLengine
{
	class Engine;
}

namespace GL_SCENE
{
	struct SetupContext;
}

namespace GL_RUNTIME
{
	struct RuntimeScenePrepareConfig;

	class RuntimeSceneSetupContextFactory
	{
	public:
		static GL_SCENE::SetupContext make(
			GLframework::AppRuntimeContext& context,
			GLengine::Engine& engine,
			const RuntimeScenePrepareConfig& config
		);
	};
}
