#pragma once

namespace GL_RUNTIME
{
	struct RuntimeApplicationShellConfig;
	struct RuntimeWindowSnapshot;

	class RuntimeApplicationEditorStartupLifecycle
	{
	public:
		static void initializeEditor(
			RuntimeApplicationShellConfig& config,
			const RuntimeWindowSnapshot& window
		);
	};
}
