#pragma once

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GL_RUNTIME
{
	struct RuntimeCameraConfig;

	class RuntimeContentCameraLifecycle
	{
	public:
		static void initializeCamera(
			GLframework::AppRuntimeContext& context,
			const RuntimeCameraConfig& config
		);
	};
}
