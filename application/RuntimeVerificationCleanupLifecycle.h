#pragma once

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GLengine
{
	class AssetSubsystem;
	class Engine;
	class RendererSubsystem;
}

namespace GL_RUNTIME
{
	struct RuntimeVerificationConfig;

	class RuntimeVerificationCleanupLifecycle
	{
	public:
		static void reportRendererSubsystemCleanup(
			GLframework::AppRuntimeContext& context,
			const GLengine::RendererSubsystem* rendererSubsystem,
			const RuntimeVerificationConfig& config
		);

		static void reportEngineCleanup(
			GLframework::AppRuntimeContext& context,
			const GLengine::Engine& engine,
			const GLengine::AssetSubsystem* assetSubsystem,
			const GLengine::RendererSubsystem* rendererSubsystem,
			const RuntimeVerificationConfig& config
		);
	};
}
