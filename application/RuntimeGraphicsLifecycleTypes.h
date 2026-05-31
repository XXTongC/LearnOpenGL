#pragma once

namespace GL_RUNTIME
{
	struct RuntimeGraphicsLifecycleConfig
	{
		int viewportWidth{ 0 };
		int viewportHeight{ 0 };
		float clearColorRed{ 0.0f };
		float clearColorGreen{ 0.0f };
		float clearColorBlue{ 0.0f };
		float clearColorAlpha{ 1.0f };
		bool reportOpenGLCapabilities{ true };
	};
}
