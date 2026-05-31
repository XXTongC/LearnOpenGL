#include "RuntimeVerificationFrameCaptureLifecycle.h"

#include "RuntimePBRRendererStatsVerification.h"
#include "RuntimeVerificationCapture.h"
#include "RuntimeVerificationConfig.h"
#include "RuntimeVerificationReport.h"

namespace GL_RUNTIME
{
	void RuntimeVerificationFrameCaptureLifecycle::captureFrameIfNeeded(
		GLframework::AppRuntimeContext& context,
		const GLengine::Engine& engine,
		const GLengine::RendererSubsystem* rendererSubsystem,
		const RuntimeVerificationConfig& config,
		unsigned int framebufferWidth,
		unsigned int framebufferHeight,
		int renderedFrameCount,
		bool& captureWritten
	)
	{
		if (!config.enabled || captureWritten || renderedFrameCount < config.captureFrame)
		{
			return;
		}

		RuntimeVerificationCapture::captureDefaultFramebuffer(
			config.capturePath,
			framebufferWidth,
			framebufferHeight
		);
		RuntimeVerificationReport::reportRenderedFrameRuntimeStats(context, engine, rendererSubsystem);
		RuntimePBRRendererStatsVerification::reportRenderedFrame(context);
		captureWritten = true;
	}
}
