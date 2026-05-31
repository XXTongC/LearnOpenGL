#pragma once

namespace GL_RUNTIME
{
	struct RuntimeWindowCallbackContext;
	struct RuntimeWindowConfig;
	struct RuntimeWindowSnapshot;

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
