#pragma once

#include "RuntimeWindowLifecycleTypes.h"

namespace GL_RUNTIME
{
	class RuntimeWindowLifecycle
	{
	public:
		static bool initialize(
			const RuntimeWindowConfig& config,
			const RuntimeWindowCallbackContext& callbackContext
		);

		static RuntimeWindowSnapshot captureSnapshot();
		static void destroy();
	};
}
