#include "RuntimeApplicationEditorStartupLifecycle.h"

#include "RuntimeApplicationConfigPolicy.h"
#include "RuntimeEditorLifecycleConfig.h"
#include "RuntimeEditorLifecycle.h"
#include "RuntimeWindowLifecycleTypes.h"

namespace GL_RUNTIME
{
	void RuntimeApplicationEditorStartupLifecycle::initializeEditor(
		RuntimeApplicationShellConfig& config,
		const RuntimeWindowSnapshot& window
	)
	{
		RuntimeEditorLifecycle::initialize(
			RuntimeApplicationConfigPolicy::makeEditorLifecycleConfig(config, window.nativeWindow)
		);
	}
}
