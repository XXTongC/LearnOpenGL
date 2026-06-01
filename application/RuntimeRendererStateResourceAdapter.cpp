#include "RuntimeRendererStateResourceAdapter.h"

#include "RuntimeRenderResourceState.h"
#include "../renderer/renderer.h"

bool GL_RUNTIME::RuntimeRendererStateResourceAdapter::syncClearColorToRenderer(
	const RuntimeRenderResourceState& renderResources
)
{
	if (!renderResources.renderer())
	{
		return false;
	}

	renderResources.renderer()->setClearColor(renderResources.clearColor());
	return true;
}

GLframework::RendererFramePassProfile* GL_RUNTIME::RuntimeRendererStateResourceAdapter::rendererFramePassProfile(
	RuntimeRenderResourceState& renderResources
)
{
	if (!renderResources.renderer())
	{
		return nullptr;
	}

	return &renderResources.renderer()->getFramePassProfile();
}
