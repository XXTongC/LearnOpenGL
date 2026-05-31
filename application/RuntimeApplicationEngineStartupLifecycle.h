#pragma once

namespace GL_RUNTIME
{
	struct RuntimeApplicationShellConfig;
	struct RuntimeApplicationState;

	class RuntimeApplicationEngineStartupLifecycle
	{
	public:
		static bool initializeEngine(
			RuntimeApplicationState& state,
			const RuntimeApplicationShellConfig& config
		);
	};
}
