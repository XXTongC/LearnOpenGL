#include "RuntimeSceneSetupResourceAdapter.h"

#include "RuntimeCameraLightState.h"
#include "RuntimeEngineAttachmentState.h"
#include "RuntimeProfileState.h"
#include "RuntimeRenderResourceState.h"
#include "RuntimeScenePrepareConfig.h"
#include "../tools/legacyExperiments/LegacyExperimentRunner.h"
#include "../tools/sceneSetup/SceneSetup.h"

GL_SCENE::SetupContext GL_RUNTIME::RuntimeSceneSetupResourceAdapter::makeSceneSetupContext(
	RuntimeRenderResourceState& renderResources,
	RuntimeCameraLightState& cameraLights,
	RuntimeProfileState& profiles,
	RuntimeEngineAttachmentState& engineAttachments,
	GLengine::Engine& engine,
	const RuntimeScenePrepareConfig& config
)
{
	return {
		renderResources.renderer(),
		renderResources.sceneOffScreen(),
		renderResources.sceneInScreen(),
		renderResources.frameRenderTargets(),
		renderResources.bloom(),
		renderResources.screenQuad(),
		renderResources.skyBoxMesh(),
		renderResources.textD(),
		renderResources.screenMaterial(),
		cameraLights.ambientLight,
		cameraLights.dirLight,
		cameraLights.spotLight,
		cameraLights.pointLights,
		config.width,
		config.height,
		config.texturePath,
		profiles.environmentProfile(),
		profiles.pbrPreviewProfile(),
		profiles.pbrLightRigProfile(),
		&engine,
		engineAttachments.engineWorld,
		engineAttachments.engineWorldEditable
	};
}

GL_EXPERIMENTS::RuntimeContext GL_RUNTIME::RuntimeSceneSetupResourceAdapter::makeLegacyExperimentContext(
	RuntimeRenderResourceState& renderResources,
	RuntimeCameraLightState& cameraLights
)
{
	return {
		renderResources.renderer(),
		renderResources.sceneOffScreen(),
		renderResources.grassMaterial(),
		renderResources.skyBoxMesh(),
		renderResources.movePlane(),
		renderResources.csmShadowMaterial(),
		cameraLights.dirLight,
		cameraLights.pointLights
	};
}
