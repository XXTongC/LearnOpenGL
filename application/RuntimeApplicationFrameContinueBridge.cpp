#include "RuntimeApplicationFrameContinueBridge.h"

#include "RuntimeApplicationConfigPolicy.h"
#include "RuntimeApplicationState.h"
#include "RuntimeFrameLifecycle.h"
#include "RuntimeFrameLifecycleConfig.h"

namespace GL_RUNTIME
{
	bool RuntimeApplicationFrameContinueBridge::shouldContinue(
		const RuntimeApplicationShellConfig& config,
		const RuntimeApplicationState& state
	)
	{
		return RuntimeFrameLifecycle::shouldContinue(
			RuntimeApplicationConfigPolicy::makeFrameLifecycleConfig(config),
			state.frameLifecycle()
		);
	}
}
