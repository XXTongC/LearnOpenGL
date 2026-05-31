#pragma once

namespace GL_RUNTIME
{
	struct RuntimeApplicationShellConfig;
	struct RuntimeApplicationState;

	class RuntimeApplicationFrameCallbackBridge
	{
	public:
		static bool shouldContinue(
			const RuntimeApplicationShellConfig& config,
			const RuntimeApplicationState& state
		);

		static void runFrame(
			RuntimeApplicationState& state,
			RuntimeApplicationShellConfig& config
		);
	};
}
