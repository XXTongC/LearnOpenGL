#pragma once

#include <string_view>

namespace GLengine
{
	class Engine;
	struct EngineDesc;
}

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GL_RUNTIME
{
	struct RuntimeEngineLifecycleState;
	struct RuntimeEngineLifecycleCleanupRefs;

	class RuntimeEngineLifecycle
	{
	public:
		static bool initializeEngine(
			GLframework::AppRuntimeContext& context,
			GLengine::Engine& engine,
			RuntimeEngineLifecycleState& state,
			const GLengine::EngineDesc& engineDesc
		);

		static bool attachRendererBackend(
			GLframework::AppRuntimeContext& context,
			RuntimeEngineLifecycleState& state,
			std::string_view rendererBackendKey
		);

		static RuntimeEngineLifecycleCleanupRefs beginCleanup(
			RuntimeEngineLifecycleState& state
		);

		static void detachRuntimeContext(
			GLframework::AppRuntimeContext& context,
			RuntimeEngineLifecycleState& state
		);

		static void shutdownEngine(GLengine::Engine& engine);
	};
}
