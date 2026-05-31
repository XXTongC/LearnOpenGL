#pragma once

namespace GL_RUNTIME
{
	struct RuntimeGraphicsLifecycleConfig;

	class RuntimeGraphicsLifecycle
	{
	public:
		static void reportWindowSetupPrompt();
		static void initializeAfterWindow(const RuntimeGraphicsLifecycleConfig& config);
	};
}
