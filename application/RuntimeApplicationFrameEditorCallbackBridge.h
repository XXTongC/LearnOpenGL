#pragma once

struct GLFWwindow;

namespace GL_RUNTIME
{
	struct RuntimeApplicationShellConfig;
	struct RuntimeApplicationState;
	struct RuntimeFrameCallbacks;

	class RuntimeApplicationFrameEditorCallbackBridge
	{
	public:
		static RuntimeFrameCallbacks makeFrameCallbacks(
			RuntimeApplicationState& state,
			RuntimeApplicationShellConfig& config,
			GLFWwindow* window
		);
	};
}
