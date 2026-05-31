#include "RuntimeApplicationStartupCallbackBridge.h"

#include "RuntimeApplicationStartupLifecycle.h"

namespace GL_RUNTIME
{
	bool RuntimeApplicationStartupCallbackBridge::initialize(
		RuntimeApplicationState& state,
		RuntimeApplicationShellConfig& config
	)
	{
		return RuntimeApplicationStartupLifecycle::initialize(state, config);
	}
}
