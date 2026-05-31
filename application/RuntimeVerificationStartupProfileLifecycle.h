#pragma once

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GL_RUNTIME
{
	struct RuntimeVerificationConfig;

	class RuntimeVerificationStartupProfileLifecycle
	{
	public:
		static void applyStartupProfile(
			GLframework::AppRuntimeContext& context,
			const RuntimeVerificationConfig& config
		);
	};
}
