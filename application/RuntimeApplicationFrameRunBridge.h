#pragma once

namespace GL_RUNTIME
{
	struct RuntimeApplicationShellConfig;
	struct RuntimeApplicationState;

	class RuntimeApplicationFrameRunBridge
	{
	public:
		static void runFrame(
			RuntimeApplicationState& state,
			RuntimeApplicationShellConfig& config
		);
	};
}
