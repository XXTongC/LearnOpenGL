#pragma once

#include <string>

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GL_RUNTIME
{
	struct RuntimePBRVerificationConfig
	{
		bool enabled{ false };
		int maxFrames{ 3 };
		int captureFrame{ 2 };
		std::string capturePath{ "out/pbr_verification.ppm" };
	};

	class RuntimePBRVerification
	{
	public:
		static void applyProfile(GLframework::AppRuntimeContext& context);
		static void reportPreparedScene(GLframework::AppRuntimeContext& context);
		static bool captureDefaultFramebuffer(
			const std::string& path,
			unsigned int width,
			unsigned int height
		);
	};
}
