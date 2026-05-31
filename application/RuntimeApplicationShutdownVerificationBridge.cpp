#include "RuntimeApplicationShutdownVerificationBridge.h"

#include "RuntimeEngineLifecycleCleanupRefs.h"
#include "RuntimeVerificationCleanupLifecycle.h"

namespace GL_RUNTIME
{
	void RuntimeApplicationShutdownVerificationBridge::reportRendererSubsystemCleanup(
		GLframework::AppRuntimeContext& context,
		const RuntimeEngineLifecycleCleanupRefs& cleanupRefs,
		const RuntimeVerificationConfig& config
	)
	{
		RuntimeVerificationCleanupLifecycle::reportRendererSubsystemCleanup(
			context,
			cleanupRefs.rendererSubsystem,
			config
		);
	}

	void RuntimeApplicationShutdownVerificationBridge::reportEngineCleanup(
		GLframework::AppRuntimeContext& context,
		const GLengine::Engine& engine,
		const RuntimeEngineLifecycleCleanupRefs& cleanupRefs,
		const RuntimeVerificationConfig& config
	)
	{
		RuntimeVerificationCleanupLifecycle::reportEngineCleanup(
			context,
			engine,
			cleanupRefs.assetSubsystem,
			cleanupRefs.rendererSubsystem,
			config
		);
	}
}
