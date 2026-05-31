#pragma once

#include "RuntimeFrameClockTypes.h"
#include "RuntimeVerificationConfig.h"

namespace GL_RUNTIME
{
	struct RuntimeFrameLifecycleConfig
	{
		bool enableGui{ true };
		RuntimeFrameClockConfig frameClock{};
		RuntimeVerificationConfig verification{};
	};
}
