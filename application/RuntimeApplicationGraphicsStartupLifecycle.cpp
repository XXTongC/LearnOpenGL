#include "RuntimeApplicationGraphicsStartupLifecycle.h"

#include "RuntimeApplicationConfigPolicy.h"
#include "RuntimeGraphicsLifecycle.h"

namespace GL_RUNTIME
{
	void RuntimeApplicationGraphicsStartupLifecycle::initializeGraphics(
		const RuntimeApplicationShellConfig& config
	)
	{
		RuntimeGraphicsLifecycle::initializeAfterWindow(
			RuntimeApplicationConfigPolicy::makeGraphicsLifecycleConfig(config)
		);
	}
}
