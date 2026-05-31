#include "RuntimeFrameLifecycle.h"

#include "Application.h"
#include "RuntimeFrameCallbacks.h"
#include "RuntimeFrameLifecycleConfig.h"
#include "RuntimeFrameLifecycleState.h"
#include "RuntimeFrameRunner.h"
#include "RuntimeFrameRunnerTypes.h"
#include "RuntimeVerificationFrameCaptureLifecycle.h"
#include "RuntimeVerificationStopPolicy.h"
#include "../engine/Engine.h"

namespace GL_RUNTIME
{
	namespace
	{
		RuntimeFrameClockConfig makeFrameClockConfig(const RuntimeFrameLifecycleConfig& config)
		{
			RuntimeFrameClockConfig frameClockConfig = config.frameClock;
			if (config.verification.enabled)
			{
				frameClockConfig.useFixedDelta = true;
				frameClockConfig.fixedDeltaSeconds = 1.0f / 60.0f;
			}
			return frameClockConfig;
		}
	}

	void RuntimeFrameLifecycle::reset(RuntimeFrameLifecycleState& state)
	{
		state.frameClock.reset();
		state.renderedFrameCount = 0;
		state.verificationCaptureWritten = false;
	}

	bool RuntimeFrameLifecycle::shouldContinue(
		const RuntimeFrameLifecycleConfig& config,
		const RuntimeFrameLifecycleState& state
	)
	{
		if (RuntimeVerificationStopPolicy::shouldStopAfterFrames(config.verification, state.renderedFrameCount))
		{
			return false;
		}

		return GL_APP->update();
	}

	void RuntimeFrameLifecycle::runFrame(
		GLframework::AppRuntimeContext& context,
		GLengine::Engine& engine,
		GLengine::RendererSubsystem* rendererSubsystem,
		GL_EXPERIMENTS::LegacyExperimentRunner& legacyExperiments,
		RuntimeFrameLifecycleState& state,
		const RuntimeFrameLifecycleConfig& config,
		unsigned int framebufferWidth,
		unsigned int framebufferHeight
	)
	{
		RuntimeFrameCallbacks callbacks{};
		runFrame(
			context,
			engine,
			rendererSubsystem,
			legacyExperiments,
			state,
			config,
			framebufferWidth,
			framebufferHeight,
			callbacks
		);
	}

	void RuntimeFrameLifecycle::runFrame(
		GLframework::AppRuntimeContext& context,
		GLengine::Engine& engine,
		GLengine::RendererSubsystem* rendererSubsystem,
		GL_EXPERIMENTS::LegacyExperimentRunner& legacyExperiments,
		RuntimeFrameLifecycleState& state,
		const RuntimeFrameLifecycleConfig& config,
		unsigned int framebufferWidth,
		unsigned int framebufferHeight,
		const RuntimeFrameCallbacks& callbacks
	)
	{
		RuntimeFrameCallbacks frameCallbacks{};
		if (config.enableGui && callbacks.renderUi)
		{
			frameCallbacks.renderUi = callbacks.renderUi;
		}

		RuntimeFrameRunner::run(
			context,
			legacyExperiments,
			{
				&engine,
				rendererSubsystem,
				framebufferWidth,
				framebufferHeight,
				state.frameClock.tick(makeFrameClockConfig(config))
			},
			frameCallbacks
		);
		++state.renderedFrameCount;

		RuntimeVerificationFrameCaptureLifecycle::captureFrameIfNeeded(
			context,
			engine,
			rendererSubsystem,
			config.verification,
			framebufferWidth,
			framebufferHeight,
			state.renderedFrameCount,
			state.verificationCaptureWritten
		);
	}
}
