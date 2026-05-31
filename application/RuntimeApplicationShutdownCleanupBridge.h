#pragma once

namespace GL_RUNTIME
{
	struct RuntimeApplicationState;
	struct RuntimeVerificationConfig;

	class RuntimeApplicationShutdownCleanupBridge
	{
	public:
		static void cleanup(
			RuntimeApplicationState& state,
			const RuntimeVerificationConfig& config
		);
	};
}
