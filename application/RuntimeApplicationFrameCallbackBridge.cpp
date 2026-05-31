#include "RuntimeApplicationFrameCallbackBridge.h"

#include "RuntimeApplicationFrameLifecycle.h"

namespace GL_RUNTIME
{
	bool RuntimeApplicationFrameCallbackBridge::shouldContinue(
		const RuntimeApplicationShellConfig& config,
		const RuntimeApplicationState& state
	)
	{
		return RuntimeApplicationFrameLifecycle::shouldContinue(config, state);
	}

	void RuntimeApplicationFrameCallbackBridge::runFrame(
		RuntimeApplicationState& state,
		RuntimeApplicationShellConfig& config
	)
	{
		RuntimeApplicationFrameLifecycle::runFrame(state, config);
	}
}
