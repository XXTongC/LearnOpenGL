#pragma once

namespace GL_RUNTIME
{
	struct RuntimeApplicationShellConfig;
	struct RuntimeApplicationState;

	class RuntimeApplicationFrameContinueBridge
	{
	public:
		static bool shouldContinue(
			const RuntimeApplicationShellConfig& config,
			const RuntimeApplicationState& state
		);
	};
}
