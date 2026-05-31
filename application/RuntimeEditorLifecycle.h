#pragma once

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GL_RUNTIME
{
	struct RuntimeFrameCallbacks;
	struct RuntimeEditorLifecycleConfig;
	struct RuntimeEditorLifecycleState;

	class RuntimeEditorLifecycle
	{
	public:
		static void initialize(const RuntimeEditorLifecycleConfig& config);

		static RuntimeFrameCallbacks makeFrameCallbacks(
			GLframework::AppRuntimeContext& context,
			RuntimeEditorLifecycleState& state,
			const RuntimeEditorLifecycleConfig& config
		);
	};
}
