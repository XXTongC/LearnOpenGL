#pragma once

#include <string_view>

namespace GLengine
{
	class RendererSubsystem;
}

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GL_RUNTIME
{
	class RuntimeRendererBackendAttachmentLifecycle
	{
	public:
		static bool attachToRendererSubsystem(
			GLframework::AppRuntimeContext& context,
			GLengine::RendererSubsystem& rendererSubsystem,
			std::string_view rendererBackendKey
		);
	};
}
