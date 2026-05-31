#pragma once

#include <string>

#include "RuntimeFrameClockTypes.h"
#include "RuntimeVerificationConfig.h"
#include "RuntimeWindowLifecycleTypes.h"

namespace GL_RUNTIME
{
	struct RuntimeApplicationShellConfig
	{
		RuntimeApplicationShellConfig();

		RuntimeWindowConfig window{ 1920, 1080 };
		std::string skyboxTexturePath{ "Texture/bk.jpg" };
		int legacyGrassRows{ 30 };
		int legacyGrassColumns{ 30 };
		float editorOrbitAngle{ 0.0f };
		bool enableGui{ true };
		std::string rendererBackendKey{};
		RuntimeFrameClockConfig frameClock{};
		RuntimeVerificationConfig verification{};
	};
}
