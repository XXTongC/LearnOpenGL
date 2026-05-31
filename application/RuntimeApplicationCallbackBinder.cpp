#include "RuntimeApplicationCallbackBinder.h"

#include "RuntimeApplicationFrameCallbackBridge.h"
#include "RuntimeApplicationShutdownCallbackBridge.h"
#include "RuntimeApplicationStartupCallbackBridge.h"
#include "RuntimeBootstrapperCallbacks.h"

namespace GL_RUNTIME
{
	RuntimeBootstrapperCallbacks RuntimeApplicationCallbackBinder::makeCallbacks(
		RuntimeApplicationState& state,
		RuntimeApplicationShellConfig& config
	)
	{
		return {
			[&state, &config]() { return RuntimeApplicationStartupCallbackBridge::initialize(state, config); },
			[&state, &config]() { return RuntimeApplicationFrameCallbackBridge::shouldContinue(config, state); },
			[&state, &config]() { RuntimeApplicationFrameCallbackBridge::runFrame(state, config); },
			[&state, &config]() { RuntimeApplicationShutdownCallbackBridge::cleanup(state, config); },
			[]() { RuntimeApplicationShutdownCallbackBridge::destroy(); }
		};
	}
}
