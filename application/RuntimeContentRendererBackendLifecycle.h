#pragma once

#include <string_view>

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GL_RUNTIME
{
	struct RuntimeEngineLifecycleState;

	class RuntimeContentRendererBackendLifecycle
	{
	public:
		static bool attachAfterScenePreparation(
			GLframework::AppRuntimeContext& context,
			RuntimeEngineLifecycleState& engineLifecycle,
			std::string_view rendererBackendKey
		);
	};
}
