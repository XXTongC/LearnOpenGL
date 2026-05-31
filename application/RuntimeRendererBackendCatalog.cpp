#include "RuntimeRendererBackendCatalog.h"

#include "RuntimeRendererBackendKeys.h"
#include "../engine/RendererBackendRegistry.h"

namespace GL_RUNTIME
{
	namespace
	{
		constexpr const char* RendererSubsystemOwnerKey = "engine-renderer-subsystem";
		constexpr const char* EngineOwnedOwnership = "engine-owned";
	}

	const char* RuntimeRendererBackendCatalog::runtimeFramePipelineBackendKey()
	{
		return RuntimeRendererBackendKeys::runtimeFramePipelineBackendKey();
	}

	const char* RuntimeRendererBackendCatalog::testNoOpBackendKey()
	{
		return RuntimeRendererBackendKeys::testNoOpBackendKey();
	}

	const char* RuntimeRendererBackendCatalog::defaultBackendKey()
	{
		return RuntimeRendererBackendKeys::defaultBackendKey();
	}

	GLengine::RendererBackendRegistry RuntimeRendererBackendCatalog::makeRegistry()
	{
		return GLengine::RendererBackendRegistry({
			{
				RuntimeRendererBackendKeys::runtimeFramePipelineBackendKey(),
				"Runtime Frame Pipeline Adapter",
				true
			},
			{
				RuntimeRendererBackendKeys::testNoOpBackendKey(),
				"Test No-op Renderer Backend",
				false
			}
		});
	}

	std::vector<GLengine::RendererBackendRegistration> RuntimeRendererBackendCatalog::registeredBackends()
	{
		auto registry = makeRegistry();
		return registry.getRegisteredBackends();
	}

	bool RuntimeRendererBackendCatalog::isRegisteredBackendKey(std::string_view backendKey)
	{
		return makeRegistry().isRegisteredBackendKey(backendKey);
	}

	GLengine::RendererBackendSelection RuntimeRendererBackendCatalog::resolveBackendSelection(std::string_view backendKey)
	{
		return makeRegistry().resolveBackendSelection(
			backendKey,
			RuntimeRendererBackendKeys::defaultBackendKey()
		);
	}

	GLengine::RendererBackendAttachmentDesc RuntimeRendererBackendCatalog::makeRendererSubsystemAttachmentDesc(
		const GLengine::RendererBackendSelection& selection
	)
	{
		return makeRegistry().makeAttachmentDesc(
			selection.selectedKey,
			RendererSubsystemOwnerKey,
			EngineOwnedOwnership
		);
	}
}
