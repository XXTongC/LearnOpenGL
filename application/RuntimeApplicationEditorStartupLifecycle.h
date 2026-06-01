#pragma once

namespace GL_RUNTIME
{
	struct RuntimeApplicationShellConfig;
	struct RuntimeApplicationState;
	struct RuntimeWindowSnapshot;

	class RuntimeApplicationEditorStartupLifecycle
	{
	public:
		static void initializeEditor(
			RuntimeApplicationState& state,
			RuntimeApplicationShellConfig& config,
			const RuntimeWindowSnapshot& window
		);
	};
}
