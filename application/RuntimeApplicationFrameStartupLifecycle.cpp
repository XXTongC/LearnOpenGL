#include "RuntimeApplicationFrameStartupLifecycle.h"

#include "RuntimeApplicationState.h"
#include "RuntimeFrameLifecycle.h"

namespace GL_RUNTIME
{
	void RuntimeApplicationFrameStartupLifecycle::resetFrameLifecycle(RuntimeApplicationState& state)
	{
		RuntimeFrameLifecycle::reset(state.frameLifecycle());
	}
}
