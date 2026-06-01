#include "RuntimeSceneSetupContextFactory.h"

#include "AppRuntimeContext.h"
#include "RuntimeScenePrepareConfig.h"
#include "RuntimeSceneSetupResourceAdapter.h"
#include "../tools/sceneSetup/SceneSetup.h"

namespace GL_RUNTIME
{
	GL_SCENE::SetupContext RuntimeSceneSetupContextFactory::make(
		GLframework::AppRuntimeContext& context,
		GLengine::Engine& engine,
		const RuntimeScenePrepareConfig& config
	)
	{
		return RuntimeSceneSetupResourceAdapter::makeSceneSetupContext(
			context.renderResources,
			context.cameraLights,
			context.profiles,
			context.engineAttachments,
			engine,
			config
		);
	}
}
