#pragma once

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GL_RUNTIME
{
	struct RuntimeCameraConfig;

	class RuntimeCameraLifecycle
	{
	public:
		static void initializeDefaultCamera(
			GLframework::AppRuntimeContext& context,
			const RuntimeCameraConfig& config
		);

		static void cleanup(GLframework::AppRuntimeContext& context);
	};
}
