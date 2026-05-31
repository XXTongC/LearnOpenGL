#pragma once

namespace GL_RUNTIME
{
	struct RuntimeApplicationState;
	struct RuntimeEngineLifecycleCleanupRefs;

	class RuntimeApplicationShutdownEngineBridge
	{
	public:
		static RuntimeEngineLifecycleCleanupRefs beginCleanup(
			RuntimeApplicationState& state
		);

		static void cleanupRuntimeContext(RuntimeApplicationState& state);

		static void shutdownEngine(RuntimeApplicationState& state);
	};
}
