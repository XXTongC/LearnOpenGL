#include "RuntimeApplicationEditorStartupLifecycle.h"

#include "RuntimeApplicationConfigPolicy.h"
#include "RuntimeApplicationState.h"
#include "RuntimeEditorLifecycleConfig.h"
#include "RuntimeEditorLifecycle.h"
#include "RuntimeWindowLifecycleTypes.h"

namespace GL_RUNTIME
{
	void RuntimeApplicationEditorStartupLifecycle::initializeEditor(
		RuntimeApplicationState& state,
		RuntimeApplicationShellConfig& config,
		const RuntimeWindowSnapshot& window
	)
	{
		RuntimeEditorLifecycle::initialize(
			state.editorLifecycle(),
			RuntimeApplicationConfigPolicy::makeEditorLifecycleConfig(config, window.nativeWindow)
		);
	}
}
