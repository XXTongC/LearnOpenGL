#pragma once

namespace GL_RUNTIME
{
	struct RuntimeApplicationShellConfig;
	struct RuntimeContentLifecycleConfig;

	class RuntimeContentConfigPolicy
	{
	public:
		static RuntimeContentLifecycleConfig makeContentLifecycleConfig(
			const RuntimeApplicationShellConfig& shellConfig,
			int framebufferWidth,
			int framebufferHeight
		);
	};
}
