#include "RuntimeFrameReadinessResourceAdapter.h"

#include "AppRuntimeContext.h"
#include "RuntimeFramePassRegistry.h"
#include "../renderer/FrameRenderTargets.h"
#include "../renderer/PostProcessSettings.h"

namespace
{
	bool isSceneColorReady(
		const GL_RUNTIME::RuntimeRenderResourceView& renderResources,
		const GLframework::AppRuntimeContext& context
	)
	{
		return renderResources.renderer() != nullptr
			&& renderResources.sceneOffScreen() != nullptr
			&& context.cameraLights.camera != nullptr
			&& renderResources.frameRenderTargets().isInitialized()
			&& renderResources.frameRenderTargets().getSceneFbo() != 0;
	}

	bool isSceneResolveReady(const GL_RUNTIME::RuntimeRenderResourceView& renderResources)
	{
		return renderResources.frameRenderTargets().getMultisample() != nullptr
			&& renderResources.frameRenderTargets().getResolved() != nullptr;
	}

	bool isBloomReady(
		const GL_RUNTIME::RuntimeRenderResourceView& renderResources,
		const GLframework::PostProcessSettings& postProcessSettings
	)
	{
		if (!postProcessSettings.bloomEnabled)
		{
			return true;
		}

		return renderResources.bloom() != nullptr
			&& renderResources.frameRenderTargets().getResolved() != nullptr
			&& renderResources.frameRenderTargets().getBloomBright() != nullptr
			&& renderResources.frameRenderTargets().getBloomPing() != nullptr
			&& renderResources.frameRenderTargets().getBloomPong() != nullptr;
	}

	bool isScreenCompositeReady(const GL_RUNTIME::RuntimeRenderResourceView& renderResources)
	{
		return renderResources.renderer() != nullptr
			&& renderResources.screenQuad() != nullptr
			&& renderResources.frameRenderTargets().isInitialized();
	}
}

bool GL_RUNTIME::RuntimeFrameReadinessResourceAdapter::isFramePassReady(
	const RuntimeFramePassDefinition& pass,
	const GLframework::AppRuntimeContext& context
)
{
	const auto renderResources = context.renderResources.readOnlyView();
	switch (pass.id)
	{
	case RuntimeFramePassId::SceneColor:
		return isSceneColorReady(renderResources, context);
	case RuntimeFramePassId::SceneResolve:
		return isSceneResolveReady(renderResources);
	case RuntimeFramePassId::Bloom:
		return isBloomReady(renderResources, context.profiles.postProcessSettings());
	case RuntimeFramePassId::ScreenComposite:
		return isScreenCompositeReady(renderResources);
	default:
		return false;
	}
}
