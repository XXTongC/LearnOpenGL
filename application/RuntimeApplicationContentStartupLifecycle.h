#pragma once

namespace GL_RUNTIME
{
	struct RuntimeApplicationShellConfig;
	struct RuntimeApplicationState;
	struct RuntimeWindowSnapshot;

	class RuntimeApplicationContentStartupLifecycle
	{
	public:
		static bool prepareContent(
			RuntimeApplicationState& state,
			const RuntimeApplicationShellConfig& config,
			const RuntimeWindowSnapshot& window
		);
	};
}
