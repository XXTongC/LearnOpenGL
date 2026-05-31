#pragma once

#include <memory>

namespace GLengine
{
	class RendererBackend;
	struct RendererBackendSelection;
}

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GL_RUNTIME
{
	class RuntimeRendererBackendFactory
	{
	public:
		static std::unique_ptr<GLengine::RendererBackend> createBackend(
			GLframework::AppRuntimeContext& context,
			const GLengine::RendererBackendSelection& selection
		);
		static std::unique_ptr<GLengine::RendererBackend> createRuntimeFramePipelineBackend(
			GLframework::AppRuntimeContext& context
		);
	};
}
