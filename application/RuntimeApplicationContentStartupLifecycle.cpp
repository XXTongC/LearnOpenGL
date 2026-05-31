#include "RuntimeApplicationContentStartupLifecycle.h"

#include "RuntimeApplicationState.h"
#include "RuntimeContentConfigPolicy.h"
#include "RuntimeContentLifecycle.h"
#include "RuntimeContentLifecycleConfig.h"
#include "RuntimeWindowLifecycleTypes.h"

namespace GL_RUNTIME
{
	bool RuntimeApplicationContentStartupLifecycle::prepareContent(
		RuntimeApplicationState& state,
		const RuntimeApplicationShellConfig& config,
		const RuntimeWindowSnapshot& window
	)
	{
		return RuntimeContentLifecycle::prepare(
			state.runtime(),
			state.engine(),
			state.engineLifecycle(),
			state.legacyExperiments(),
			RuntimeContentConfigPolicy::makeContentLifecycleConfig(
				config,
				window.width,
				window.height
			)
		);
	}
}
