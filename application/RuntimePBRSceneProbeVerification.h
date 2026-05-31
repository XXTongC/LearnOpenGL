#pragma once

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GL_RUNTIME
{
	struct RuntimeVerificationConfig;

	class RuntimePBRSceneProbeVerification
	{
	public:
		static void addVerificationSceneProbes(
			GLframework::AppRuntimeContext& context,
			const RuntimeVerificationConfig& config
		);
	};
}
