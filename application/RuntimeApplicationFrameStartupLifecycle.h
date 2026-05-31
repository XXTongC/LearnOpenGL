#pragma once

namespace GL_RUNTIME
{
	struct RuntimeApplicationState;

	class RuntimeApplicationFrameStartupLifecycle
	{
	public:
		static void resetFrameLifecycle(RuntimeApplicationState& state);
	};
}
