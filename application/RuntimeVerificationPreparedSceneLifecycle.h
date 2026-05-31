#pragma once

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GL_RUNTIME
{
	struct RuntimeVerificationConfig;

	class RuntimeVerificationPreparedSceneLifecycle
	{
	public:
		static void reportPreparedScene(
			GLframework::AppRuntimeContext& context,
			const RuntimeVerificationConfig& config
		);
	};
}
