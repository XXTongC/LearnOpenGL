#include "RuntimeContentRendererBackendLifecycle.h"

#include "AppRuntimeContext.h"
#include "RuntimeEngineLifecycle.h"
#include "../tools/Logger/LogManager.h"

namespace GL_RUNTIME
{
	bool RuntimeContentRendererBackendLifecycle::attachAfterScenePreparation(
		GLframework::AppRuntimeContext& context,
		RuntimeEngineLifecycleState& engineLifecycle,
		std::string_view rendererBackendKey
	)
	{
		if (!context.renderResources.renderer)
		{
			LogError("Runtime content preparation failed: scene prepare did not create a renderer");
			return false;
		}

		return RuntimeEngineLifecycle::attachRendererBackend(context, engineLifecycle, rendererBackendKey);
	}
}
