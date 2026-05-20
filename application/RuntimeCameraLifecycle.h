#pragma once

#include "AppRuntimeContext.h"

namespace GL_RUNTIME
{
	struct RuntimeCameraConfig
	{
		int width{ 1 };
		int height{ 1 };
		float fovy{ 60.0f };
		float nearPlane{ 0.1f };
		float farPlane{ 1000.0f };
	};

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
