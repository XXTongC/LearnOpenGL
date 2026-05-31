#include "RuntimeApplicationShutdownCleanupBridge.h"

#include "RuntimeApplicationShutdownEngineBridge.h"
#include "RuntimeApplicationShutdownVerificationBridge.h"
#include "RuntimeApplicationState.h"
#include "RuntimeEngineLifecycleCleanupRefs.h"

namespace GL_RUNTIME
{
	void RuntimeApplicationShutdownCleanupBridge::cleanup(
		RuntimeApplicationState& state,
		const RuntimeVerificationConfig& config
	)
	{
		const RuntimeEngineLifecycleCleanupRefs cleanupRefs =
			RuntimeApplicationShutdownEngineBridge::beginCleanup(state);
		RuntimeApplicationShutdownVerificationBridge::reportRendererSubsystemCleanup(
			state.runtime(),
			cleanupRefs,
			config
		);
		RuntimeApplicationShutdownEngineBridge::cleanupRuntimeContext(state);
		RuntimeApplicationShutdownEngineBridge::shutdownEngine(state);
		RuntimeApplicationShutdownVerificationBridge::reportEngineCleanup(
			state.runtime(),
			state.engine(),
			cleanupRefs,
			config
		);
	}
}
