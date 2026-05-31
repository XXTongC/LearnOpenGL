#include "RuntimeApplicationFrameEditorCallbackBridge.h"

#include "RuntimeApplicationConfigPolicy.h"
#include "RuntimeApplicationState.h"
#include "RuntimeEditorLifecycleConfig.h"
#include "RuntimeEditorLifecycle.h"
#include "RuntimeFrameCallbacks.h"

namespace GL_RUNTIME
{
	RuntimeFrameCallbacks RuntimeApplicationFrameEditorCallbackBridge::makeFrameCallbacks(
		RuntimeApplicationState& state,
		RuntimeApplicationShellConfig& config,
		GLFWwindow* window
	)
	{
		return RuntimeEditorLifecycle::makeFrameCallbacks(
			state.runtime(),
			state.editorLifecycle(),
			RuntimeApplicationConfigPolicy::makeEditorLifecycleConfig(config, window)
		);
	}
}
