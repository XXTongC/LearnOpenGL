#include "RuntimeApplicationFrameRunBridge.h"

#include "RuntimeApplicationConfigPolicy.h"
#include "RuntimeApplicationFrameEditorCallbackBridge.h"
#include "RuntimeApplicationState.h"
#include "RuntimeEngineLifecycleState.h"
#include "RuntimeFrameCallbacks.h"
#include "RuntimeFrameLifecycle.h"
#include "RuntimeFrameLifecycleConfig.h"
#include "RuntimeWindowLifecycle.h"

namespace GL_RUNTIME
{
	void RuntimeApplicationFrameRunBridge::runFrame(
		RuntimeApplicationState& state,
		RuntimeApplicationShellConfig& config
	)
	{
		const RuntimeWindowSnapshot window = RuntimeWindowLifecycle::captureSnapshot();
		RuntimeFrameLifecycle::runFrame(
			state.runtime(),
			state.engine(),
			state.engineLifecycle().rendererSubsystem,
			state.legacyExperiments(),
			state.frameLifecycle(),
			RuntimeApplicationConfigPolicy::makeFrameLifecycleConfig(config),
			static_cast<unsigned int>(window.width),
			static_cast<unsigned int>(window.height),
			RuntimeApplicationFrameEditorCallbackBridge::makeFrameCallbacks(state, config, window.nativeWindow)
		);
	}
}
