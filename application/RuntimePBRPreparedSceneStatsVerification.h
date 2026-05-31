#pragma once

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GL_RUNTIME
{
	struct RuntimeVerificationConfig;

	class RuntimePBRPreparedSceneStatsVerification
	{
	public:
		static void reportPreparedScene(
			GLframework::AppRuntimeContext& context,
			const RuntimeVerificationConfig& config
		);
	};
}
