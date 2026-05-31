#pragma once

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GL_RUNTIME
{
	struct RuntimeVerificationConfig;

	class RuntimeEngineWorldVerification
	{
	public:
		static int countRuntimeWorldActors(const GLframework::AppRuntimeContext& context);
		static void addVerificationSceneProbes(
			GLframework::AppRuntimeContext& context,
			const RuntimeVerificationConfig& config
		);
		static void reportPreparedScene(
			GLframework::AppRuntimeContext& context,
			const RuntimeVerificationConfig& config
		);
	};
}
