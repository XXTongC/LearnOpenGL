#include "RuntimeSceneSetupContextFactory.h"

#include "AppRuntimeContext.h"
#include "RuntimeScenePrepareConfig.h"
#include "../tools/sceneSetup/SceneSetup.h"

namespace GL_RUNTIME
{
	GL_SCENE::SetupContext RuntimeSceneSetupContextFactory::make(
		GLframework::AppRuntimeContext& context,
		GLengine::Engine& engine,
		const RuntimeScenePrepareConfig& config
	)
	{
		return {
			context.renderResources.renderer,
			context.renderResources.sceneOffScreen,
			context.renderResources.sceneInScreen,
			context.renderResources.frameRenderTargets(),
			context.renderResources.bloom,
			context.renderResources.screenQuad,
			context.renderResources.skyBoxMesh,
			context.renderResources.textD,
			context.renderResources.screenMaterial,
			context.cameraLights.ambientLight,
			context.cameraLights.dirLight,
			context.cameraLights.spotLight,
			context.cameraLights.pointLights,
			config.width,
			config.height,
			config.texturePath,
			context.profiles.environmentProfile(),
			context.profiles.pbrPreviewProfile,
			context.profiles.pbrLightRigProfile,
			&engine,
			context.engineAttachments.engineWorld,
			context.engineAttachments.engineWorldEditable
		};
	}
}
