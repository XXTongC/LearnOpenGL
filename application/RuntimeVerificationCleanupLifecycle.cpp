#include "RuntimeVerificationCleanupLifecycle.h"

#include "RuntimeVerificationConfig.h"
#include "RuntimeVerificationReport.h"

namespace GL_RUNTIME
{
	void RuntimeVerificationCleanupLifecycle::reportRendererSubsystemCleanup(
		GLframework::AppRuntimeContext& context,
		const GLengine::RendererSubsystem* rendererSubsystem,
		const RuntimeVerificationConfig& config
	)
	{
		if (!config.enabled || !rendererSubsystem)
		{
			return;
		}

		RuntimeVerificationReport::reportRendererSubsystemCleanup(context, rendererSubsystem);
	}

	void RuntimeVerificationCleanupLifecycle::reportEngineCleanup(
		GLframework::AppRuntimeContext& context,
		const GLengine::Engine& engine,
		const GLengine::AssetSubsystem* assetSubsystem,
		const GLengine::RendererSubsystem* rendererSubsystem,
		const RuntimeVerificationConfig& config
	)
	{
		if (!config.enabled)
		{
			return;
		}

		RuntimeVerificationReport::reportEngineWorldCleanup(context, engine);
		RuntimeVerificationReport::reportEngineSubsystemCleanup(context, engine, assetSubsystem, rendererSubsystem);
	}
}
