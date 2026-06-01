#include "RuntimeRendererBackendResourceAdapter.h"

#include "RuntimeRenderResourceState.h"
#include "../engine/RendererSubsystem.h"

bool GL_RUNTIME::RuntimeRendererBackendResourceAdapter::hasRuntimeRenderer(
	const RuntimeRenderResourceState& renderResources
)
{
	return renderResources.renderer() != nullptr;
}

void GL_RUNTIME::RuntimeRendererBackendResourceAdapter::attachRuntimeRenderer(
	GLengine::RendererSubsystem& rendererSubsystem,
	const RuntimeRenderResourceState& renderResources
)
{
	rendererSubsystem.setRenderer(renderResources.renderer().get());
}

bool GL_RUNTIME::RuntimeRendererBackendResourceAdapter::isRuntimeRendererAttached(
	const GLengine::RendererSubsystem& rendererSubsystem,
	const RuntimeRenderResourceState& renderResources
)
{
	return rendererSubsystem.getRenderer() == renderResources.renderer().get();
}
