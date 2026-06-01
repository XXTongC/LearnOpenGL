#include "RuntimeRendererBackendAttachmentLifecycle.h"

#include <utility>

#include "AppRuntimeContext.h"
#include "RuntimeRendererBackendCatalog.h"
#include "RuntimeRendererBackendFactory.h"
#include "../engine/RendererBackend.h"
#include "../engine/RendererSubsystem.h"

namespace GL_RUNTIME
{
	bool RuntimeRendererBackendAttachmentLifecycle::attachToRendererSubsystem(
		GLframework::AppRuntimeContext& context,
		GLengine::RendererSubsystem& rendererSubsystem,
		std::string_view rendererBackendKey
	)
	{
		const auto renderResources = context.renderResources.readOnlyView();
		rendererSubsystem.setRenderer(renderResources.renderer().get());

		const auto selection = RuntimeRendererBackendCatalog::resolveBackendSelection(rendererBackendKey);
		auto rendererBackend = RuntimeRendererBackendFactory::createBackend(context, selection);
		if (!rendererBackend)
		{
			return false;
		}

		rendererSubsystem.setRendererBackend(
			std::move(rendererBackend),
			RuntimeRendererBackendCatalog::makeRendererSubsystemAttachmentDesc(selection)
		);
		return true;
	}
}
