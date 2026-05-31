#pragma once

namespace GL_RUNTIME
{
	struct RuntimeApplicationShellConfig;
	struct RuntimeApplicationState;

	class RuntimeApplicationStartupLifecycle
	{
	public:
		static bool initialize(
			RuntimeApplicationState& state,
			RuntimeApplicationShellConfig& config
		);
	};
}
