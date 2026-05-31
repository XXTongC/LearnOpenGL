#pragma once

struct GLFWwindow;

namespace GLengine
{
	struct EngineDesc;
}

namespace GL_RUNTIME
{
	struct RuntimeApplicationShellConfig;
	struct RuntimeEditorLifecycleConfig;
	struct RuntimeFrameLifecycleConfig;
	struct RuntimeGraphicsLifecycleConfig;
	struct RuntimeVerificationConfig;

	class RuntimeApplicationConfigPolicy
	{
	public:
		static GLengine::EngineDesc makeEngineDesc(const RuntimeApplicationShellConfig& shellConfig);
		static RuntimeFrameLifecycleConfig makeFrameLifecycleConfig(const RuntimeApplicationShellConfig& shellConfig);
		static RuntimeEditorLifecycleConfig makeEditorLifecycleConfig(
			RuntimeApplicationShellConfig& shellConfig,
			GLFWwindow* window
		);
		static RuntimeGraphicsLifecycleConfig makeGraphicsLifecycleConfig(const RuntimeApplicationShellConfig& shellConfig);
		static const RuntimeVerificationConfig& verificationConfig(const RuntimeApplicationShellConfig& shellConfig);
	};
}
