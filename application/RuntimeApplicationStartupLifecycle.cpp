#include "RuntimeApplicationStartupLifecycle.h"

#include "RuntimeApplicationContentStartupLifecycle.h"
#include "RuntimeApplicationEditorStartupLifecycle.h"
#include "RuntimeApplicationEngineStartupLifecycle.h"
#include "RuntimeApplicationFrameStartupLifecycle.h"
#include "RuntimeApplicationGraphicsStartupLifecycle.h"
#include "RuntimeApplicationWindowStartupLifecycle.h"
#include "RuntimeWindowLifecycleTypes.h"

namespace GL_RUNTIME
{
	bool RuntimeApplicationStartupLifecycle::initialize(
		RuntimeApplicationState& state,
		RuntimeApplicationShellConfig& config
	)
	{
		if (!RuntimeApplicationEngineStartupLifecycle::initializeEngine(state, config))
		{
			return false;
		}

		if (!RuntimeApplicationWindowStartupLifecycle::initializeWindow(state, config))
		{
			return false;
		}

		RuntimeApplicationGraphicsStartupLifecycle::initializeGraphics(config);

		const RuntimeWindowSnapshot window =
			RuntimeApplicationWindowStartupLifecycle::captureWindowSnapshot();
		if (!RuntimeApplicationContentStartupLifecycle::prepareContent(state, config, window))
		{
			return false;
		}

		RuntimeApplicationEditorStartupLifecycle::initializeEditor(state, config, window);
		RuntimeApplicationFrameStartupLifecycle::resetFrameLifecycle(state);
		return true;
	}
}
