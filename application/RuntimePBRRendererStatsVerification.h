#pragma once

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GL_RUNTIME
{
	class RuntimePBRRendererStatsVerification
	{
	public:
		static void reportRenderedFrame(GLframework::AppRuntimeContext& context);
	};
}
