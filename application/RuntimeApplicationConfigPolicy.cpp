#include "RuntimeApplicationConfigPolicy.h"

#include "RuntimeApplicationConfig.h"
#include "RuntimeEditorLifecycleConfig.h"
#include "RuntimeFrameLifecycleConfig.h"
#include "RuntimeGraphicsLifecycle.h"
#include "../engine/EngineDesc.h"

namespace GL_RUNTIME
{
	GLengine::EngineDesc RuntimeApplicationConfigPolicy::makeEngineDesc(
		const RuntimeApplicationShellConfig& shellConfig
	)
	{
		GLengine::EngineDesc desc{};
		desc.runMode = shellConfig.verification.enabled
			? GLengine::EngineRunMode::Verification
			: GLengine::EngineRunMode::Editor;
		desc.viewportWidth = shellConfig.window.width;
		desc.viewportHeight = shellConfig.window.height;
		return desc;
	}

	RuntimeFrameLifecycleConfig RuntimeApplicationConfigPolicy::makeFrameLifecycleConfig(
		const RuntimeApplicationShellConfig& shellConfig
	)
	{
		RuntimeFrameLifecycleConfig config{};
		config.enableGui = shellConfig.enableGui;
		config.frameClock = shellConfig.frameClock;
		config.verification = shellConfig.verification;
		return config;
	}

	RuntimeEditorLifecycleConfig RuntimeApplicationConfigPolicy::makeEditorLifecycleConfig(
		RuntimeApplicationShellConfig& shellConfig,
		GLFWwindow* window
	)
	{
		return {
			shellConfig.enableGui,
			window,
			&shellConfig.editorOrbitAngle
		};
	}

	RuntimeGraphicsLifecycleConfig RuntimeApplicationConfigPolicy::makeGraphicsLifecycleConfig(
		const RuntimeApplicationShellConfig& shellConfig
	)
	{
		RuntimeGraphicsLifecycleConfig config{};
		config.viewportWidth = shellConfig.window.width;
		config.viewportHeight = shellConfig.window.height;
		return config;
	}

	const RuntimeVerificationConfig& RuntimeApplicationConfigPolicy::verificationConfig(
		const RuntimeApplicationShellConfig& shellConfig
	)
	{
		return shellConfig.verification;
	}
}
