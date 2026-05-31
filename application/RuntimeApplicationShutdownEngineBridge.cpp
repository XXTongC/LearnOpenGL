#include "RuntimeApplicationShutdownEngineBridge.h"

#include "RuntimeApplicationState.h"
#include "RuntimeCameraLifecycle.h"
#include "RuntimeEngineLifecycle.h"
#include "RuntimeEngineLifecycleCleanupRefs.h"

namespace GL_RUNTIME
{
	RuntimeEngineLifecycleCleanupRefs RuntimeApplicationShutdownEngineBridge::beginCleanup(
		RuntimeApplicationState& state
	)
	{
		return RuntimeEngineLifecycle::beginCleanup(state.engineLifecycle());
	}

	void RuntimeApplicationShutdownEngineBridge::cleanupRuntimeContext(RuntimeApplicationState& state)
	{
		RuntimeCameraLifecycle::cleanup(state.runtime());
		RuntimeEngineLifecycle::detachRuntimeContext(state.runtime(), state.engineLifecycle());
	}

	void RuntimeApplicationShutdownEngineBridge::shutdownEngine(RuntimeApplicationState& state)
	{
		RuntimeEngineLifecycle::shutdownEngine(state.engine());
	}
}
