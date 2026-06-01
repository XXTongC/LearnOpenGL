#pragma once

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GL_RUNTIME
{
	struct RuntimeFrameCallbacks;
	struct RuntimeEditorLifecycleConfig;
	class RuntimeEditorLifecycleState;

	class RuntimeEditorLifecycle
	{
	public:
		static void initialize(
			RuntimeEditorLifecycleState& state,
			const RuntimeEditorLifecycleConfig& config
		);

		static RuntimeFrameCallbacks makeFrameCallbacks(
			GLframework::AppRuntimeContext& context,
			RuntimeEditorLifecycleState& state,
			const RuntimeEditorLifecycleConfig& config
		);
	};
}
