#include "RuntimeWindowRenderResourceAdapter.h"

#include "RuntimeRenderResourceState.h"
#include "RuntimeViewport.h"

bool GL_RUNTIME::RuntimeWindowRenderResourceAdapter::applyResize(
	RuntimeRenderResourceState& renderResources,
	Camera* camera,
	int* width,
	int* height,
	int newWidth,
	int newHeight
)
{
	const auto result = RuntimeViewport::applyResize(
		newWidth,
		newHeight,
		{
			width,
			height,
			camera,
			&renderResources.frameRenderTargets(),
			renderResources.screenMaterial()
		}
	);
	return result.accepted;
}
