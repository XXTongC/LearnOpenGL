#pragma once

namespace GL_RUNTIME
{
	struct RuntimeApplicationShellConfig;
	struct RuntimeApplicationState;

	class RuntimeApplicationShutdownCallbackBridge
	{
	public:
		static void cleanup(
			RuntimeApplicationState& state,
			const RuntimeApplicationShellConfig& config
		);

		static void destroy();
	};
}
