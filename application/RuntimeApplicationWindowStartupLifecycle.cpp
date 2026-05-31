#include "RuntimeApplicationWindowStartupLifecycle.h"

#include "RuntimeApplicationConfig.h"
#include "RuntimeApplicationState.h"
#include "RuntimeGraphicsLifecycle.h"
#include "RuntimeWindowLifecycle.h"
#include "RuntimeWindowLifecycleTypes.h"

namespace GL_RUNTIME
{
	bool RuntimeApplicationWindowStartupLifecycle::initializeWindow(
		RuntimeApplicationState& state,
		RuntimeApplicationShellConfig& config
	)
	{
		RuntimeGraphicsLifecycle::reportWindowSetupPrompt();
		return RuntimeWindowLifecycle::initialize(
			config.window,
			{ &state.runtime(), &config.window.width, &config.window.height }
		);
	}

	RuntimeWindowSnapshot RuntimeApplicationWindowStartupLifecycle::captureWindowSnapshot()
	{
		return RuntimeWindowLifecycle::captureSnapshot();
	}
}
