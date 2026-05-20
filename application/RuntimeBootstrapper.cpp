#include "RuntimeBootstrapper.h"

int GL_RUNTIME::RuntimeBootstrapper::run(const RuntimeBootstrapperCallbacks& callbacks)
{
	if (!callbacks.initialize || !callbacks.shouldContinue || !callbacks.runFrame)
	{
		return -1;
	}

	if (!callbacks.initialize())
	{
		return -1;
	}

	while (callbacks.shouldContinue())
	{
		callbacks.runFrame();
	}

	if (callbacks.cleanup)
	{
		callbacks.cleanup();
	}

	if (callbacks.destroy)
	{
		callbacks.destroy();
	}

	return 0;
}
