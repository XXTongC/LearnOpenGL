#pragma once

namespace GL_RUNTIME
{
	struct RuntimeApplicationShellConfig;
	struct RuntimeApplicationState;

	class RuntimeApplicationStartupCallbackBridge
	{
	public:
		static bool initialize(
			RuntimeApplicationState& state,
			RuntimeApplicationShellConfig& config
		);
	};
}
