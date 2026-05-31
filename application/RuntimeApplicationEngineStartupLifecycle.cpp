#include "RuntimeApplicationEngineStartupLifecycle.h"

#include "RuntimeApplicationConfigPolicy.h"
#include "RuntimeApplicationState.h"
#include "RuntimeEngineLifecycle.h"
#include "../engine/EngineDesc.h"

namespace GL_RUNTIME
{
	bool RuntimeApplicationEngineStartupLifecycle::initializeEngine(
		RuntimeApplicationState& state,
		const RuntimeApplicationShellConfig& config
	)
	{
		return RuntimeEngineLifecycle::initializeEngine(
			state.runtime(),
			state.engine(),
			state.engineLifecycle(),
			RuntimeApplicationConfigPolicy::makeEngineDesc(config)
		);
	}
}
