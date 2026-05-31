#pragma once

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GLengine
{
	class Engine;
	class RendererSubsystem;
}

namespace GL_RUNTIME
{
	struct RuntimeVerificationConfig;

	class RuntimeVerificationFrameCaptureLifecycle
	{
	public:
		static void captureFrameIfNeeded(
			GLframework::AppRuntimeContext& context,
			const GLengine::Engine& engine,
			const GLengine::RendererSubsystem* rendererSubsystem,
			const RuntimeVerificationConfig& config,
			unsigned int framebufferWidth,
			unsigned int framebufferHeight,
			int renderedFrameCount,
			bool& captureWritten
		);
	};
}
