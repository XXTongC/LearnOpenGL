#include "RuntimeFrameRenderResourceAdapter.h"

#include "RuntimeCameraLightState.h"
#include "RuntimeRenderResourceState.h"
#include "../materials/MaterialTypes.h"
#include "../renderer/FrameRenderTargets.h"
#include "../renderer/PostProcessPass.h"
#include "../renderer/PostProcessSettings.h"
#include "../renderer/renderer.h"

void GL_RUNTIME::RuntimeFrameRenderResourceAdapter::renderSceneColor(
	RuntimeRenderResourceState& renderResources,
	const RuntimeCameraLightState& cameraLights
)
{
	renderResources.renderer()->render(
		renderResources.sceneOffScreen(),
		cameraLights.camera,
		cameraLights.dirLight,
		cameraLights.spotLight,
		cameraLights.pointLights,
		cameraLights.ambientLight,
		renderResources.frameRenderTargets().getSceneFbo()
	);
}

void GL_RUNTIME::RuntimeFrameRenderResourceAdapter::resolveSceneColor(
	RuntimeRenderResourceState& renderResources
)
{
	renderResources.postProcessPass().resolveMultisample(
		renderResources.frameRenderTargets().getMultisample(),
		renderResources.frameRenderTargets().getResolved()
	);
}

void GL_RUNTIME::RuntimeFrameRenderResourceAdapter::renderBloom(
	RuntimeRenderResourceState& renderResources,
	const GLframework::PostProcessSettings& postProcessSettings
)
{
	if (!postProcessSettings.bloomEnabled)
	{
		return;
	}

	renderResources.postProcessPass().extractBloomBright(
		renderResources.bloom(),
		renderResources.frameRenderTargets().getResolved(),
		renderResources.frameRenderTargets().getBloomBright(),
		postProcessSettings.bloomThreshold
	);
	renderResources.postProcessPass().blurBloom(
		renderResources.bloom(),
		renderResources.frameRenderTargets().getBloomBright(),
		renderResources.frameRenderTargets().getBloomPing(),
		renderResources.frameRenderTargets().getBloomPong(),
		postProcessSettings.bloomIterations
	);
}

void GL_RUNTIME::RuntimeFrameRenderResourceAdapter::renderScreenComposite(
	RuntimeRenderResourceState& renderResources,
	const GLframework::PostProcessSettings& postProcessSettings,
	int framebufferWidth,
	int framebufferHeight
)
{
	renderResources.postProcessPass().renderScreenComposite(
		renderResources.screenQuad(),
		renderResources.renderer()->getShader(GLframework::MaterialType::ScreenMaterial),
		postProcessSettings,
		framebufferWidth,
		framebufferHeight
	);
}
