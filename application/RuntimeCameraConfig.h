#pragma once

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
}
