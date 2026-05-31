#include "RuntimeApplicationShutdownLifecycle.h"

#include "RuntimeApplicationShutdownCleanupBridge.h"
#include "RuntimeApplicationShutdownDestroyBridge.h"

namespace GL_RUNTIME
{
	void RuntimeApplicationShutdownLifecycle::cleanup(
		RuntimeApplicationState& state,
		const RuntimeVerificationConfig& config
	)
	{
		RuntimeApplicationShutdownCleanupBridge::cleanup(state, config);
	}

	void RuntimeApplicationShutdownLifecycle::destroy()
	{
		RuntimeApplicationShutdownDestroyBridge::destroy();
	}
}
