#pragma once

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GL_RUNTIME
{
	struct RuntimeVerificationConfig;

	class RuntimePBRPreviewProfileVerification
	{
	public:
		static void applyPreviewProfile(
			GLframework::AppRuntimeContext& context,
			const RuntimeVerificationConfig& verification
		);
	};
}
