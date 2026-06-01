#pragma once

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GL_RUNTIME
{
	struct RuntimeFramePassDefinition;

	class RuntimeFrameReadinessResourceAdapter
	{
	public:
		static bool isFramePassReady(
			const RuntimeFramePassDefinition& pass,
			const GLframework::AppRuntimeContext& context
		);
	};
}
