#pragma once

#include <string>

#include "RuntimeCameraConfig.h"
#include "RuntimeScenePrepareConfig.h"
#include "RuntimeVerificationConfig.h"

namespace GL_RUNTIME
{
	struct RuntimeContentLifecycleConfig
	{
		RuntimeCameraConfig camera{};
		RuntimeScenePrepareConfig scene{};
		RuntimeVerificationConfig verification{};
		std::string rendererBackendKey{};
	};
}
