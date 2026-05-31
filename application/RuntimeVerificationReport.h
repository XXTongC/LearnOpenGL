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
	class RuntimeVerificationReport
	{
	public:
		static void reportRenderedFrameRuntimeStats(
			GLframework::AppRuntimeContext& context,
			const GLengine::Engine& engine,
			const GLengine::RendererSubsystem* rendererSubsystem
		);

		static void reportRendererSubsystemCleanup(
			GLframework::AppRuntimeContext& context,
			const GLengine::RendererSubsystem* rendererSubsystem
		);

		static void reportEngineWorldCleanup(
			GLframework::AppRuntimeContext& context,
			const GLengine::Engine& engine
		);

		static void reportEngineSubsystemCleanup(
			GLframework::AppRuntimeContext& context,
			const GLengine::Engine& engine,
			const GLengine::AssetSubsystem* assetSubsystem,
			const GLengine::RendererSubsystem* rendererSubsystem
		);
	};
}
