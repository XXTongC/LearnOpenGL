#pragma once

#include "AppRuntimeContext.h"

namespace GL_RUNTIME
{
	struct RuntimeWindowConfig
	{
		int width{ 1200 };
		int height{ 900 };
	};

	struct RuntimeWindowCallbackContext
	{
		GLframework::AppRuntimeContext* runtime{ nullptr };
		int* width{ nullptr };
		int* height{ nullptr };
	};

	class RuntimeWindowLifecycle
	{
	public:
		static bool initialize(
			const RuntimeWindowConfig& config,
			const RuntimeWindowCallbackContext& callbackContext
		);
	};
}
