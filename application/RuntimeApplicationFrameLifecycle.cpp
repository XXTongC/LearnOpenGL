#include "RuntimeApplicationFrameLifecycle.h"

#include "RuntimeApplicationFrameContinueBridge.h"
#include "RuntimeApplicationFrameRunBridge.h"

namespace GL_RUNTIME
{
	bool RuntimeApplicationFrameLifecycle::shouldContinue(
		const RuntimeApplicationShellConfig& config,
		const RuntimeApplicationState& state
	)
	{
		return RuntimeApplicationFrameContinueBridge::shouldContinue(config, state);
	}

	void RuntimeApplicationFrameLifecycle::runFrame(
		RuntimeApplicationState& state,
		RuntimeApplicationShellConfig& config
	)
	{
		RuntimeApplicationFrameRunBridge::runFrame(state, config);
	}
}
