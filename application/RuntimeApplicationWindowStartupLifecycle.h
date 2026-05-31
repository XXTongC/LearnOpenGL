#pragma once

namespace GL_RUNTIME
{
	struct RuntimeApplicationShellConfig;
	struct RuntimeApplicationState;
	struct RuntimeWindowSnapshot;

	class RuntimeApplicationWindowStartupLifecycle
	{
	public:
		static bool initializeWindow(
			RuntimeApplicationState& state,
			RuntimeApplicationShellConfig& config
		);

		static RuntimeWindowSnapshot captureWindowSnapshot();
	};
}
