#pragma once

#include "RuntimeFrameClock.h"

namespace GL_RUNTIME
{
	struct RuntimeFrameLifecycleState
	{
		RuntimeFrameClock frameClock{};
		int renderedFrameCount{ 0 };
		bool verificationCaptureWritten{ false };
	};
}
