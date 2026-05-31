#include "RuntimeRendererBackendFactory.h"

#include "RuntimeNoOpRendererBackend.h"
#include "RuntimeRendererBackendKeys.h"
#include "RuntimeRendererFrameBridgeAdapter.h"
#include "../engine/RendererBackendRegistryTypes.h"

namespace GL_RUNTIME
{
	std::unique_ptr<GLengine::RendererBackend> RuntimeRendererBackendFactory::createBackend(
		GLframework::AppRuntimeContext& context,
		const GLengine::RendererBackendSelection& selection
	)
	{
		if (!selection.registered)
		{
			return nullptr;
		}

		if (selection.selectedKey == RuntimeRendererBackendKeys::runtimeFramePipelineBackendKey())
		{
			return createRuntimeFramePipelineBackend(context);
		}
		if (selection.selectedKey == RuntimeRendererBackendKeys::testNoOpBackendKey())
		{
			return std::make_unique<RuntimeNoOpRendererBackend>();
		}

		return nullptr;
	}

	std::unique_ptr<GLengine::RendererBackend> RuntimeRendererBackendFactory::createRuntimeFramePipelineBackend(
		GLframework::AppRuntimeContext& context
	)
	{
		return std::make_unique<RuntimeRendererFrameBridgeAdapter>(context);
	}
}
