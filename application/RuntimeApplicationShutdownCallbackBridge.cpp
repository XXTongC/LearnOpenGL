#include "RuntimeApplicationShutdownCallbackBridge.h"

#include "RuntimeApplicationConfigPolicy.h"
#include "RuntimeApplicationShutdownLifecycle.h"

namespace GL_RUNTIME
{
	void RuntimeApplicationShutdownCallbackBridge::cleanup(
		RuntimeApplicationState& state,
		const RuntimeApplicationShellConfig& config
	)
	{
		RuntimeApplicationShutdownLifecycle::cleanup(
			state,
			RuntimeApplicationConfigPolicy::verificationConfig(config)
		);
	}

	void RuntimeApplicationShutdownCallbackBridge::destroy()
	{
		RuntimeApplicationShutdownLifecycle::destroy();
	}
}
