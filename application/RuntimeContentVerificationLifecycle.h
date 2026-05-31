#pragma once

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GL_RUNTIME
{
	struct RuntimeVerificationConfig;

	class RuntimeContentVerificationLifecycle
	{
	public:
		static void loadStartupProfiles(
			GLframework::AppRuntimeContext& context,
			const RuntimeVerificationConfig& config
		);

		static void reportPreparedScene(
			GLframework::AppRuntimeContext& context,
			const RuntimeVerificationConfig& config
		);
	};
}
