#pragma once

#include <string_view>
#include <vector>

#include "../engine/RendererBackendRegistryTypes.h"

namespace GL_RUNTIME
{
	class RuntimeRendererBackendCatalog
	{
	public:
		static const char* runtimeFramePipelineBackendKey();
		static const char* testNoOpBackendKey();
		static const char* defaultBackendKey();
		static std::vector<GLengine::RendererBackendRegistration> registeredBackends();
		static bool isRegisteredBackendKey(std::string_view backendKey);
		static GLengine::RendererBackendSelection resolveBackendSelection(std::string_view backendKey);
		static GLengine::RendererBackendAttachmentDesc makeRendererSubsystemAttachmentDesc(
			const GLengine::RendererBackendSelection& selection
		);
	};
}
