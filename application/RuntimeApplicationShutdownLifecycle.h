#pragma once

namespace GL_RUNTIME
{
	struct RuntimeApplicationState;
	struct RuntimeVerificationConfig;

	class RuntimeApplicationShutdownLifecycle
	{
	public:
		static void cleanup(
			RuntimeApplicationState& state,
			const RuntimeVerificationConfig& config
		);

		static void destroy();
	};
}
