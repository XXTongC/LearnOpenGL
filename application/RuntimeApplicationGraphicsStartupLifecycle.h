#pragma once

namespace GL_RUNTIME
{
	struct RuntimeApplicationShellConfig;

	class RuntimeApplicationGraphicsStartupLifecycle
	{
	public:
		static void initializeGraphics(const RuntimeApplicationShellConfig& config);
	};
}
