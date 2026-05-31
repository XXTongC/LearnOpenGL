#pragma once

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GL_RUNTIME
{
	struct RuntimeVerificationConfig;

	class RuntimePBRLightCameraRigVerification
	{
	public:
		static void applyLightCameraRig(
			GLframework::AppRuntimeContext& context,
			const RuntimeVerificationConfig& verification
		);
	};
}
