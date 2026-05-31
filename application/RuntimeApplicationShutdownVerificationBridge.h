#pragma once

namespace GLengine
{
	class Engine;
}

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GL_RUNTIME
{
	struct RuntimeEngineLifecycleCleanupRefs;
	struct RuntimeVerificationConfig;

	class RuntimeApplicationShutdownVerificationBridge
	{
	public:
		static void reportRendererSubsystemCleanup(
			GLframework::AppRuntimeContext& context,
			const RuntimeEngineLifecycleCleanupRefs& cleanupRefs,
			const RuntimeVerificationConfig& config
		);

		static void reportEngineCleanup(
			GLframework::AppRuntimeContext& context,
			const GLengine::Engine& engine,
			const RuntimeEngineLifecycleCleanupRefs& cleanupRefs,
			const RuntimeVerificationConfig& config
		);
	};
}
