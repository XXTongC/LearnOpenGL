#include "RuntimeApplicationShell.h"

#include <iostream>
#include <utility>

#include "Application.h"
#include "GL_ERROR_FIND.h"
#include "RuntimeCameraLifecycle.h"
#include "RuntimeEditorPanelCoordinator.h"
#include "RuntimeFrameRunner.h"
#include "RuntimeGuiHost.h"
#include "RuntimeProfileLoader.h"
#include "RuntimeScenePreparer.h"
#include "RuntimeViewport.h"
#include "core.h"

namespace GL_RUNTIME
{
	RuntimeApplicationShell::RuntimeApplicationShell(RuntimeApplicationShellConfig config)
		: mConfig(std::move(config))
	{
	}

	RuntimeBootstrapperCallbacks RuntimeApplicationShell::makeCallbacks()
	{
		return {
			[this]() { return initialize(); },
			[this]() { return shouldContinue(); },
			[this]() { runFrame(); },
			[this]() { cleanup(); },
			[this]() { destroy(); }
		};
	}

	bool RuntimeApplicationShell::initialize()
	{
		std::cout << "Please set the window as x * y" << std::endl;
		if (!RuntimeWindowLifecycle::initialize(
			mConfig.window,
			{ &mRuntime, &mConfig.window.width, &mConfig.window.height }
		)) return false;

		RuntimeViewport::applyViewport(mConfig.window.width, mConfig.window.height);
		GL_CALL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));

		RuntimeCameraLifecycle::initializeDefaultCamera(mRuntime, makeCameraConfig());
		RuntimeProfileLoader::loadAll(mRuntime);
		if (mConfig.pbrVerification.enabled)
		{
			RuntimePBRVerification::applyProfile(mRuntime);
		}
		RuntimeScenePreparer::prepare(mRuntime, mLegacyExperiments, makeScenePrepareConfig());
		if (mConfig.pbrVerification.enabled)
		{
			RuntimePBRVerification::reportPreparedScene(mRuntime);
		}
		if (mConfig.enableGui)
		{
			RuntimeGuiHost::initialize({ GL_APP->getWindow() });
		}
		printOpenGLCapabilities();

		return true;
	}

	bool RuntimeApplicationShell::shouldContinue()
	{
		if (mConfig.pbrVerification.enabled && mRenderedFrameCount >= mConfig.pbrVerification.maxFrames)
		{
			return false;
		}

		return GL_APP->update();
	}

	void RuntimeApplicationShell::runFrame()
	{
		RuntimeFrameCallbacks callbacks{};
		if (mConfig.enableGui)
		{
			callbacks.renderUi = [this]() { renderFrameUi(); };
		}

		RuntimeFrameRunner::run(
			mRuntime,
			mLegacyExperiments,
			makeFrameConfig(),
			callbacks
		);
		++mRenderedFrameCount;
		captureVerificationFrameIfNeeded();
	}

	void RuntimeApplicationShell::cleanup()
	{
		RuntimeCameraLifecycle::cleanup(mRuntime);
	}

	void RuntimeApplicationShell::destroy()
	{
		GL_APP->destroy();
	}

	RuntimeFrameConfig RuntimeApplicationShell::makeFrameConfig() const
	{
		return {
			static_cast<unsigned int>(GL_APP->getWidth()),
			static_cast<unsigned int>(GL_APP->getHeight())
		};
	}

	RuntimeCameraConfig RuntimeApplicationShell::makeCameraConfig() const
	{
		return {
			static_cast<int>(GL_APP->getWidth()),
			static_cast<int>(GL_APP->getHeight())
		};
	}

	RuntimeScenePrepareConfig RuntimeApplicationShell::makeScenePrepareConfig() const
	{
		return {
			mConfig.window.width,
			mConfig.window.height,
			mConfig.skyboxTexturePath,
			mConfig.legacyGrassRows,
			mConfig.legacyGrassColumns
		};
	}

	void RuntimeApplicationShell::renderFrameUi()
	{
		RuntimeGuiHost::renderFrame({ GL_APP->getWindow(), [this]() { drawEditorPanels(); } });
	}

	void RuntimeApplicationShell::drawEditorPanels()
	{
		RuntimeEditorPanelCoordinator::drawPanels(mRuntime, mEditorSelection, &mConfig.editorOrbitAngle);
	}

	void RuntimeApplicationShell::captureVerificationFrameIfNeeded()
	{
		if (
			!mConfig.pbrVerification.enabled ||
			mVerificationCaptureWritten ||
			mRenderedFrameCount < mConfig.pbrVerification.captureFrame
		)
		{
			return;
		}

		RuntimePBRVerification::captureDefaultFramebuffer(
			mConfig.pbrVerification.capturePath,
			static_cast<unsigned int>(GL_APP->getWidth()),
			static_cast<unsigned int>(GL_APP->getHeight())
		);
		RuntimePBRVerification::reportRenderedFrame(mRuntime);
		mVerificationCaptureWritten = true;
	}

	void RuntimeApplicationShell::printOpenGLCapabilities() const
	{
		int nrAttributes = 0;
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
		std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes << std::endl;
	}
}
